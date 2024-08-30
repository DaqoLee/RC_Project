#ifndef ENUMCODE_H
#define ENUMCODE_H
enum ERRORCODE
{
    NORMAL = 0,
    NOINSTALLSDK,
    NODEVICES,
    OUTOFRANGE,
    PARAMETERERR,
    COLLECTIONCYCLEDATALOSS,
    CREATEFFECTERR,
    ENCODINGFAILED,
    FFBERR,
};

enum PRODUCTTYPE
{
    PRODUCT_WHEELBASE = 0,
    PRODUCT_STEERINGWHEEL,
    PRODUCT_DISPLAYSCREEN,
    PRODUCT_PEDALS,
    PRODUCT_METER,
    PRODUCT_ADAPTER,
    PRODUCT_HANDBRAKE,
    PRODUCT_GEARSHIFTER,
    PRODUCT_UNKNOWDEVICE,
};
#endif // BASESINGLETON_H
