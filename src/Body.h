// Body: the physical state every actor in a simulation has. Reconstructed from SQZ.EXE 0x40bb70-0x40bc60 and 0x41fb90-0x4200fd.
//
// A body holds where its creature is and which way it faces: a 4x4 matrix, the matrix's facing row copied out, three turn angles,
// and the "contact": which surface face the creature stands on and where it is relative to that face. The classes that are bodies
// for particular creatures (SqueezilBody, GoodieBody, CameraBody) add their own state after it and fill in the pure functions.
//
// The objects are made and used by original code through the vtable at +0 (like the props, see VirtualCall.h), so the functions
// the original calls through it are plain members here, found through the original table (slot numbers in BodySlot).
#pragma once
#include "Float3.h"

// How a body stands on the surface it is on (0x48 bytes)
struct Contact {
    int   face;          // 0x00 the surface face, -1 for none
    int   second;        // 0x04 (not identified; starts 0)
    float matrix[16];    // 0x08 the creature's place relative to that face
};
static_assert(sizeof(Contact) == 0x48, "Contact layout");

enum BodySlot {
    BODY_DESTROY = 0, BODY_GET_MATRIX = 3, BODY_GET_FACING = 4, BODY_GET_FACING_VALUE = 5, BODY_GET_EXTRA_VALUE = 6,
    BODY_GET_CONTACT = 7, BODY_SET_MATRIX = 9, BODY_SET_FACING = 10, BODY_SET_ANGLES = 11, BODY_GET_ANGLES = 12,
    BODY_PLACE_ON_FACE = 13, BODY_SET_CONTACT = 14
};

struct Body {
    void **vtbl;            // 0x00 @0049d484 for the base class
    float  matrix[16];      // 0x04
    Float3 facing;          // 0x44 starts as the matrix's third row
    float  facingValue;     // 0x50 set with the facing (role not identified; starts 0)
    Float3 angles;          // 0x54 three turn angles (radians)
    float  unused60[3];     // 0x60 never written by the base class
    float  extraValue;      // 0x6c (role not identified; starts 0)
    Contact contact;        // 0x70
    // 0xb8: the derived classes' own state (the simulation reads a flag word there)

    // @0041fb90 the matrix is copied from m; angles start at zero and the contact at "no face"
    Body *Init(const float *m);

    // The base class's virtual functions (the small ones are 4 bytes long in the original and stay there)
    float *GetMatrix()          { return matrix; }           // @0041fcb0 slot 3
    Float3 *GetFacing()         { return &facing; }          // @0040bb70 slot 4
    float  GetFacingValue()     { return facingValue; }      // @0040bb80 slot 5
    float  GetExtraValue()      { return extraValue; }       // @0040bb90 slot 6
    Contact *GetContact()       { return &contact; }         // @0040bba0 slot 7
    void   SetMatrix(const float *m);                        // @0041fcc0 slot 9 (the facing is not updated)
    void   SetFacing(const Float3 *direction, float value);  // @0041fd30 slot 10
    void   SetAngles(float x, float y, float z);             // @0041fd60 slot 11
    void   GetAngles(float *x, float *y, float *z);          // @0041fd80 slot 12
    void   SetContact(const Contact *c);                     // @00420060 slot 14
};
static_assert(sizeof(Body) == 0xb8, "Body layout");
