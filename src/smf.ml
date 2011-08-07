type error =
    SmfNullReturn

exception Error of error

let () =
  Callback.register_exception "smf_exn_error" (Error SmfNullReturn)

type t
type track
type event_t
type event = {
        (*track : track;*)
        event_number : int;
	delta_time_pulses : int;
        time_pulses : int;
        time_seconds : float;
        track_number : int;
	midi_buffer : (int, Bigarray.int8_unsigned_elt, Bigarray.c_layout) Bigarray.Array1.t;
        event : event_t;
	(*void		*user_pointer;*)
}

external load : string -> t = "ocaml_smf_load"
external get_next_event : t -> event = "ocaml_smf_get_next_event"
external event_is_metadata : event -> bool = "ocaml_smf_event_is_metadata"
external smf_new : unit -> t = "ocaml_smf_new"
external track_new : unit -> track = "ocaml_smf_track_new"
external add_track : t -> track -> unit = "ocaml_smf_add_track"
external event_new_from_pointer : (int, Bigarray.int8_unsigned_elt, Bigarray.c_layout) Bigarray.Array1.t -> int -> event = "ocaml_smf_event_new_from_pointer"
external event_new_from_bytes : int -> int -> int -> event = "ocaml_smf_event_new_from_bytes"
external delete_event : event -> unit = "ocaml_smf_event_delete"
external track_add_event_seconds : track -> event -> int -> unit = "ocaml_smf_track_add_event_seconds"
external save : t -> string -> bool = "ocaml_smf_save"
external get_track_by_number : t -> int -> track = "ocaml_smf_get_track_by_number"
external track_get_event_by_number : track -> int -> event = "ocaml_smf_track_get_event_by_number"
