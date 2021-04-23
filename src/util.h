#ifndef UTIL_JS
#define UTIL_JS

#include <algorithm>
#include <limits>
#include <nan.h>
#include <openvr.h>
#include <v8.h>

//=========================================================
// C++ -> Node
template <typename T>
v8::Local<v8::Value> encode(const T &value);

// Node -> C++
template <typename T>
T decode(const v8::Local<v8::Value>, v8::Isolate *);

//=========================================================
template <>
v8::Local<v8::Value> encode(const vr::HmdMatrix44_t &value)
{
    Nan::EscapableHandleScope scope;
    auto result = Nan::New<v8::Array>();

    for (uint32_t rowIdx = 0; rowIdx < 4; ++rowIdx)
    {
        auto row = Nan::New<v8::Array>();
        for (uint32_t colIdx = 0; colIdx < 4; ++colIdx)
            Nan::Set(row, colIdx, Nan::New<v8::Number>(value.m[rowIdx][colIdx]));
        Nan::Set(result, rowIdx, row);
    }

    return scope.Escape(result);
}

//=========================================================
template <>
v8::Local<v8::Value> encode(const vr::HmdMatrix34_t &value)
{
    Nan::EscapableHandleScope scope;
    auto result = Nan::New<v8::Array>();

    for (uint32_t rowIdx = 0; rowIdx < 3; ++rowIdx)
    {
        auto row = Nan::New<v8::Array>();
        for (uint32_t colIdx = 0; colIdx < 4; ++colIdx)
            Nan::Set(row, colIdx, Nan::New<v8::Number>(value.m[rowIdx][colIdx]));
        Nan::Set(result, rowIdx, row);
    }

    return scope.Escape(result);
}

//=========================================================
template <>
vr::HmdMatrix34_t decode(const v8::Local<v8::Value> value, v8::Isolate *isolate)
{
    vr::HmdMatrix34_t result;
    const auto matrix = value->ToObject(isolate->GetCurrentContext()).ToLocalChecked();

    for (uint32_t rowIdx = 0; rowIdx < 3; ++rowIdx)
    {
        const auto row = Nan::Get(matrix, rowIdx).ToLocalChecked()->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
        for (uint32_t colIdx = 0; colIdx < 4; ++colIdx)
        {
            const auto cell = Nan::Get(row, colIdx).ToLocalChecked();
            result.m[rowIdx][colIdx] = static_cast<float>(cell->NumberValue(isolate->GetCurrentContext()).FromJust());
        }
    }

    return result;
}

//=========================================================
template <>
v8::Local<v8::Value> encode(const vr::HmdVector3_t &value)
{
    Nan::EscapableHandleScope scope;
    auto result = Nan::New<v8::Array>();

    for (uint32_t idx = 0; idx < 3; ++idx)
        Nan::Set(result, idx, Nan::New<v8::Number>(value.v[idx]));

    return scope.Escape(result);
}

//=========================================================
template <>
vr::HmdVector3_t decode(const v8::Local<v8::Value> value, v8::Isolate *isolate)
{
    vr::HmdVector3_t result;
    const auto array = value->ToObject(isolate->GetCurrentContext()).ToLocalChecked();

    for (uint32_t idx = 0; idx < 3; ++idx)
    {
        const auto cell = Nan::Get(array, idx).ToLocalChecked();
        result.v[idx] = static_cast<float>(cell->NumberValue(isolate->GetCurrentContext()).FromJust());
    }

    return result;
}

//=========================================================
template <>
v8::Local<v8::Value> encode(const vr::DistortionCoordinates_t &value)
{
    Nan::EscapableHandleScope scope;
    auto result = Nan::New<v8::Object>();

    // Extract red values.
    {
        auto red = Nan::New<v8::Array>();
        Nan::Set(red, 0, Nan::New<v8::Number>(value.rfRed[0]));
        Nan::Set(red, 1, Nan::New<v8::Number>(value.rfRed[1]));

        auto red_prop = Nan::New<v8::String>("red").ToLocalChecked();
        Nan::Set(result, red_prop, red);
    }

    // Extract green values.
    {
        auto green = Nan::New<v8::Array>();
        Nan::Set(green, 0, Nan::New<v8::Number>(value.rfGreen[0]));
        Nan::Set(green, 1, Nan::New<v8::Number>(value.rfGreen[1]));

        auto green_prop = Nan::New<v8::String>("green").ToLocalChecked();
        Nan::Set(result, green_prop, green);
    }

    // Extract blue values.
    {
        auto blue = Nan::New<v8::Array>();
        Nan::Set(blue, 0, Nan::New<v8::Number>(value.rfBlue[0]));
        Nan::Set(blue, 1, Nan::New<v8::Number>(value.rfBlue[1]));

        auto blue_prop = Nan::New<v8::String>("blue").ToLocalChecked();
        Nan::Set(result, blue_prop, blue);
    }

    return scope.Escape(result);
}

//=========================================================
template <>
v8::Local<v8::Value> encode(const vr::TrackedDeviceIndex_t &value)
{
    return Nan::New<v8::Number>(value);
}

template <>
vr::TrackedDevicePose_t decode(const v8::Local<v8::Value> value, v8::Isolate *isolate)
{
    vr::TrackedDevicePose_t result;
    const auto object = value->ToObject(isolate->GetCurrentContext()).ToLocalChecked();

    auto deviceToAbsoluteTracking_prop =
        Nan::New<v8::String>("deviceToAbsoluteTracking").ToLocalChecked();
    result.mDeviceToAbsoluteTracking =
        decode<vr::HmdMatrix34_t>(
            Nan::Get(object, deviceToAbsoluteTracking_prop).ToLocalChecked(), isolate);

    auto velocity_prop =
        Nan::New<v8::String>("velocity").ToLocalChecked();
    result.vVelocity =
        decode<vr::HmdVector3_t>(Nan::Get(object, velocity_prop).ToLocalChecked(), isolate);

    auto angularVelocity_prop =
        Nan::New<v8::String>("angularVelocity").ToLocalChecked();
    result.vAngularVelocity =
        decode<vr::HmdVector3_t>(
            Nan::Get(object, angularVelocity_prop).ToLocalChecked(), isolate);

    auto trackingResult_prop =
        Nan::New<v8::String>("trackingResult").ToLocalChecked();
    result.eTrackingResult =
        static_cast<vr::ETrackingResult>(
            Nan::Get(object, trackingResult_prop).ToLocalChecked()->Uint32Value(isolate->GetCurrentContext()).FromJust());

    auto poseIsValid_prop =
        Nan::New<v8::String>("poseIsValid").ToLocalChecked();
    result.bPoseIsValid =
        Nan::Get(object, poseIsValid_prop).ToLocalChecked()->BooleanValue(isolate);

    auto deviceIsConnected_prop =
        Nan::New<v8::String>("deviceIsConnected").ToLocalChecked();
    result.bDeviceIsConnected =
        Nan::Get(object, deviceIsConnected_prop).ToLocalChecked()->BooleanValue(isolate);

    return result;
}

template <typename T>
v8::Local<v8::Value> encode(const T &value, uint32_t nDeviceIndices)
{
    Nan::EscapableHandleScope scope;
    auto result = Nan::New<v8::Array>();

    const uint32_t numElements = std::min(static_cast<uint32_t>(value.size()), nDeviceIndices);
    for (uint32_t idx = 0; idx < numElements; ++idx)
        Nan::Set(result, idx, encode(value[idx]));

    return scope.Escape(result);
}

template <typename T>
v8::Local<v8::Value> encode(const T &value)
{
    return encode(value, value.size());
}

#endif