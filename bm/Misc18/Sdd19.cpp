// SQZ.EXE 0x40b910 (CameraBody) and 0x450910 (GrantMessageMap): scalar deleting destructors that only run the base destructor
// (0x40bae0 and 0x40b340 respectively).
struct BCam { virtual ~BCam(); };
struct CameraBody : BCam {
    CameraBody();
};
CameraBody::CameraBody() {}

struct BGrant { virtual ~BGrant(); };
struct GrantMessageMap : BGrant {
    GrantMessageMap();
};
GrantMessageMap::GrantMessageMap() {}
