#ifndef __JSON_h__
#define __JSON_h__

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include <string>
#include <vector>
#include "Math/float3.h"
#include "Math/float4.h"
#include "Math/Quat.h"

class JSON_value
{
public:
	JSON_value(rapidjson::Document::AllocatorType* allocator, rapidjson::Type type = rapidjson::kObjectType);
	~JSON_value();

	JSON_value * CreateValue(rapidjson::Type type = rapidjson::kObjectType);

	ENGINE_API void AddInt(const char* name, int value);
	ENGINE_API void AddUint(const char * name, unsigned value);
	ENGINE_API void AddFloat(const char * name, float value);
	ENGINE_API void AddFloat2(const char * name, float2 value);
	ENGINE_API void AddFloat3(const char * name, float3 value);
	ENGINE_API void AddFloat4(const char * name, float4 value);
	ENGINE_API void AddFloat4x4(const char * name, float4x4 value);
	ENGINE_API void AddQuat(const char * name, Quat value);
	ENGINE_API void AddString(const char * name, const char* value);
	ENGINE_API void AddValue(const char * name, const JSON_value &value);

	ENGINE_API int GetInt(const char* name) const;
	ENGINE_API unsigned GetUint(const char * name) const;
	ENGINE_API float GetFloat(const char * namee) const;
	ENGINE_API float2 GetFloat2(const char * name) const;
	ENGINE_API float3 GetFloat3(const char * name) const;
	ENGINE_API float3 GetColor3(const char * name) const;
	ENGINE_API float4 GetFloat4(const char * name) const;
	ENGINE_API float4 GetColor4(const char * name) const;
	ENGINE_API float4x4 GetFloat4x4(const char * name) const;
	ENGINE_API Quat GetQuat(const char * name) const;
	ENGINE_API const char* GetString(const char * name) const;

	ENGINE_API JSON_value* GetValue(unsigned index); //Allocates value
	ENGINE_API JSON_value * GetValue(const char * name); //Allocates value
	ENGINE_API unsigned Size() const;

public:
	rapidjson::Value *rapidjsonValue = nullptr;
	rapidjson::Document::AllocatorType* allocator = nullptr;
private:
	std::vector<JSON_value*> valuesAllocated;
};

class JSON
{
public:
	JSON();
	JSON(const char * data);
	~JSON();

	JSON_value* CreateValue(rapidjson::Type type = rapidjson::kObjectType);
	void AddValue(const char * name, const JSON_value & value);
	JSON_value * GetValue(const char * name);  //Allocates value
	std::string ToString() const;
	unsigned Size(); //Clears old buffer

private:
	rapidjson::Document *document = nullptr;
	rapidjson::Document::AllocatorType* allocator = nullptr;
	rapidjson::StringBuffer *jsonBuffer = nullptr;
	std::vector<JSON_value*> valuesAllocated;
};

#endif __JSON_h__
