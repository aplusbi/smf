#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/custom.h>
#include <caml/fail.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/signals.h>
#include <caml/bigarray.h>

#include <smf.h>

static int smf_err(int i)
{
  caml_raise_with_arg(*caml_named_value("smf_exn_error"), Val_int(i));
}

typedef struct
{
    smf_t *t;
} Smf_t;

#define Smf_t_val(v) (*((Smf_t**)Data_custom_val(v)))
#define Smf_val(v) (Smf_t_val(v))->t

static void finalize_smf(value s)
{
  Smf_t *t = Smf_t_val(s);

  if (t->t)
    smf_delete(t->t);
  free(t);
}

static struct custom_operations smf_ops =
{
    "caml_smf_t",
    finalize_smf,
    custom_compare_default,
    custom_hash_default,
    custom_serialize_default,
    custom_deserialize_default
};

typedef struct
{
    smf_track_t *t;
} Track_t;

#define Track_t_val(v) (*((Track_t**)Data_custom_val(v)))
#define Track_val(v) (Track_t_val(v))->t

static void finalize_track(value s)
{
  Track_t *t = Track_t_val(s);
  free(t);
}

static struct custom_operations track_ops =
{
    "caml_track_t",
    finalize_track,
    custom_compare_default,
    custom_hash_default,
    custom_serialize_default,
    custom_deserialize_default
};

typedef struct
{
    smf_event_t *t;
} Event_t;

#define Event_t_val(v) (*((Event_t**)Data_custom_val(v)))
#define Event_val(v) (Event_t_val(v))->t

static void finalize_event(value s)
{
  Event_t *t = Event_t_val(s);
  free(t);
}

static struct custom_operations event_ops =
{
    "caml_event_t",
    finalize_event,
    custom_compare_default,
    custom_hash_default,
    custom_serialize_default,
    custom_deserialize_default
};

value create_event(smf_event_t *event)
{
    CAMLparam0();
    CAMLlocal2(ret, ans);
    Event_t *et;
    ret = caml_alloc_tuple(7);
    Field(ret, 0) = Val_int(event->event_number);
    Field(ret, 1) = Val_int(event->delta_time_pulses);
    Field(ret, 2) = Val_int(event->time_pulses);
    Field(ret, 3) = caml_copy_double(event->time_seconds);
    Field(ret, 4) = Val_int(event->track_number);
    Field(ret, 5) = caml_ba_alloc_dims(CAML_BA_UINT8 | CAML_BA_C_LAYOUT, 1, (void*)event->midi_buffer, event->midi_buffer_length, NULL);
    et = malloc(sizeof(Event_t));
    et->t = event;
    ans = caml_alloc_custom(&event_ops, sizeof(Event_t*), 1, 0);
    Event_t_val(ans) = et;
    Field(ret, 6) = ans;

    CAMLreturn(ret);
}

smf_event_t *get_event(value event)
{
    return Event_val(Field(event, 6));
}

CAMLprim value ocaml_smf_load(value file)
{
    CAMLparam1(file);
    CAMLlocal1(ans);
    Smf_t *t;
    t = malloc(sizeof(Smf_t));
    t->t = smf_load(String_val(file));
    if(t->t == NULL)
    {
        free(t);
        t = NULL;
        smf_err(0);
    }
    ans = caml_alloc_custom(&smf_ops, sizeof(Smf_t*), 1, 0);
    Smf_t_val(ans) = t;

    CAMLreturn(ans);
}

CAMLprim value ocaml_smf_get_next_event(value smf)
{
    CAMLparam1(smf);
    CAMLlocal1(ret);
    smf_event_t *event;
    event = smf_get_next_event(Smf_val(smf));
    if(event == NULL)
    {
        smf_err(0);
    }

    ret = create_event(event);

    CAMLreturn(ret);
}

CAMLprim value ocaml_smf_event_is_metadata(value event)
{
    CAMLparam1(event);
    int ret;
    smf_event_t *e = get_event(event);
    ret = smf_event_is_metadata(e);
    CAMLreturn(Val_bool(ret));
}

CAMLprim value ocaml_smf_new(value unit)
{
    CAMLparam1(unit);
    CAMLlocal1(ans);
    Smf_t *t;
    t = malloc(sizeof(Smf_t));
    t->t = smf_new();
    if(t->t == NULL)
    {
        free(t);
        /* Error */
        t = NULL;
        smf_err(0);
    }
    ans = caml_alloc_custom(&smf_ops, sizeof(Smf_t*), 1, 0);
    Smf_t_val(ans) = t;

    CAMLreturn(ans);
}

CAMLprim value ocaml_smf_track_new(value unit)
{
    CAMLparam1(unit);
    CAMLlocal1(ans);
    Track_t *t;
    t = malloc(sizeof(Track_t));
    t->t = smf_track_new();
    if(t->t == NULL)
    {
        free(t);
        /* Error */
        t = NULL;
        smf_err(0);
    }
    ans = caml_alloc_custom(&track_ops, sizeof(Track_t*), 1, 0);
    Track_t_val(ans) = t;

    CAMLreturn(ans);
}

CAMLprim value ocaml_smf_add_track(value smf, value track)
{
    CAMLparam2(smf, track);
    smf_add_track(Smf_val(smf), Track_val(track));
    CAMLreturn(Val_unit);
}

CAMLprim value ocaml_smf_event_new_from_pointer(value msg, value length)
{
    CAMLparam2(msg, length);
    CAMLlocal1(ret);
    smf_event_t *event;
    event = smf_event_new_from_pointer(Caml_ba_data_val(msg), Int_val(length));
    if(event == NULL)
    {
        smf_err(0);
    }

    ret = create_event(event);

    CAMLreturn(ret);
}

CAMLprim value ocaml_smf_event_new_from_bytes(value byte0, value byte1, value byte2)
{
    CAMLparam3(byte0, byte1, byte2);
    CAMLlocal1(ret);
    smf_event_t *event;
    event = smf_event_new_from_bytes(Int_val(byte0), Int_val(byte1), Int_val(byte2));
    if(event == NULL)
    {
        smf_err(0);
    }

    ret = create_event(event);

    CAMLreturn(ret);
}

CAMLprim value ocaml_smf_event_delete(value event)
{
    CAMLparam1(event);
    smf_event_delete(get_event(event));
    CAMLreturn(Val_unit);
}

CAMLprim value ocaml_smf_track_add_event_seconds(value track, value event, value seconds)
{
    CAMLparam3(track, event, seconds);
    float secs;
    secs = (float)Int_val(seconds);
    secs /= 1000.0f;
    smf_track_add_event_seconds(Track_val(track), get_event(event), secs);
    CAMLreturn(Val_unit);
}

CAMLprim value ocaml_smf_save(value smf, value file)
{
    CAMLparam2(smf, file);
    int ret;
    ret = smf_save(Smf_val(smf), String_val(file));
    CAMLreturn(Val_bool(ret));
}

CAMLprim value ocaml_smf_get_track_by_number(value smf, value trackno)
{
    CAMLparam2(smf, trackno);
    CAMLlocal1(ret);
    Track_t *t;
    t = malloc(sizeof(Track_t));
    t->t = smf_get_track_by_number(Smf_val(smf), Int_val(trackno));
    if(t->t == NULL)
    {
        free(t);
        /* Error */
        t = NULL;
        smf_err(0);
    }
    ret = caml_alloc_custom(&track_ops, sizeof(Track_t*), 1, 0);
    Track_t_val(ret) = t;
    CAMLreturn(ret);
}

