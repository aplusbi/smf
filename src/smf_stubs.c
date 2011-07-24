#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/custom.h>
#include <caml/fail.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/signals.h>
#include <caml/bigarray.h>

#include <smf.h>

typedef struct
{
    smf_t *t;
} Smf_t;

#define Smf_t_val(v) (*((Smf_t**)Data_custom_val(v)))
#define Smf_val(v) (Smf_t_val(v))->stream

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
#define Track_val(v) (Track_t_val(v))->stream

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

CAMLprim value caml_smf_load(value file)
{
    CAMLparam1(file);
    CAMLlocal1(ans);
    Smf_t *t;
    t = malloc(sizeof(Smf_t));
    t->t = smf_load(String_val(file));
    if(t->t == NULL)
    {
        free(t);
        /* Error */
        t = NULL;
    }
    ans = caml_alloc_custom(&smf_ops, sizeof(Smf_t*), 1, 0);
    Smf_t_val(ans) = t;

    CAMLreturn(ans);
}

CAMLprim value caml_smf_get_next_event(value smf)
{
    CAMLparam1(smf);
    CAMLlocal1(ret);
    smf_event_t *event;
    event = smf_get_next_event(Smf_t_val(smf));

    ret = caml_alloc_tuple(6);
    Field(ret, 0) = Val_int(event->event_number);
    Field(ret, 1) = Val_int(event->delta_time_pulses);
    Field(ret, 2) = Val_int(event->time_pulses);
    Field(ret, 3) = caml_copy_double(event->time_seconds);
    Field(ret, 4) = Val_int(event->track_number);
    Field(ret, 5) = caml_ba_alloc_dims(CAML_BA_UINT8 | CAML_BA_C_LAYOUT, 1, (void*)event->midi_buffer, event->midi_buffer_length, NULL);

    CAMLreturn(event);
}

