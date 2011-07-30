type mode = Input | Output

let get_device m =
    print_endline "Select device:";
    let print_device i = function
        | Input, {Portmidi.input=true; Portmidi.name=n}
        | Output, {Portmidi.output=true; Portmidi.name=n} ->
                print_int i;
                print_endline (" " ^ n);
        | _ -> ()
    in
    let count = Portmidi.count_devices () in
    for i = 0 to count - 1 do
        let info = Portmidi.get_device_info i in
        print_device i (m, info);
        ()
    done;
    read_int ()

let write_midi st event =
    let {Smf.midi_buffer=buffer} = event in
    let size = (Bigarray.Array1.dim buffer) in
    let msg = match size with
    | 2 ->
            let a, b = (Bigarray.Array1.get buffer 0), (Bigarray.Array1.get buffer 1) in
            let str = (string_of_int a) ^ " " ^ (string_of_int b) ^ "\n" in
            ignore (Unix.write Unix.stdout str 0 (String.length str));
            Portmidi.message a b 0
    | 3 ->
            let a, b, c  = (Bigarray.Array1.get buffer 0), (Bigarray.Array1.get buffer 1), (Bigarray.Array1.get buffer 2) in
            let str = (string_of_int a) ^ " " ^ (string_of_int b) ^ " " ^ (string_of_int c) ^ "\n" in
            ignore (Unix.write Unix.stdout str 0 (String.length str));
            Portmidi.message a b c
    | _ -> Printf.printf "Oops\n"; Int32.zero
    in
    Portmidi.write_short st Int32.zero msg

let _ =
    Portmidi.init ();
    Portmidi.Time.start 1;
    let d = get_device Output in
    let st = Portmidi.open_output d 8 0 in
    let smf = Smf.load "test/bach_gavotte.mid" in
    while true do
        let event = Smf.get_next_event smf in
        match Smf.event_is_metadata event with
        | true -> ()
        | false ->
                begin
                    let {Smf.time_pulses=wait} = event in
                    let wait = wait / 100 in
                    let str = (string_of_int wait) ^ "\n" in
                    ignore (Unix.write Unix.stdout str 0 (String.length str));
                    Portmidi.Time.sleep wait;
                    write_midi st event;
                    ()
                end;
        ()
    done