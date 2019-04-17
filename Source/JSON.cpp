#include "ModuleTextures.h"
#include "JSON.h"

#include "Globals.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "Math/float4x4.h"
#include "Math/float2.h"

JSON_value::JSON_value(rapidjson::Document::AllocatorType * allocator, rapidjson::Type type): allocator(allocator)
{
	rapidjsonValue = new rapidjson::Value(type);
}

JSON_value::~JSON_value()
{
	RELEASE(rapidjsonValue);
	for (auto value : valuesAllocated)
	{
		RELEASE(value);
	}
}

JSON_value* JSON_value::CreateValue(rapidjson::Type type)
{
	JSON_value* ret = new JSON_value(allocator, type);
	valuesAllocated.push_back(ret);
	return ret;
}

void JSON_value::AddInt(const char * name, int value)
{
	std::string myname(name);
	rapidjson::Value key(myname.c_str(), myname.size(), *allocator);
	rapidjsonValue->AddMember(key, value, *allocator);
}

void JSON_value::AddUint(const char * name, unsigned value)
{
	std::string myname(name);
	rapidjson::Value key(myname.c_str(), myname.size(), *allocator);
	rapidjsonValue->AddMember(key, value, *allocator);
}

void JSON_value::AddFloat(const char * name, float value)
{
	std::string myname(name);
	rapidjson::Value key(myname.c_str(), myname.size(), *allocator);
	rapidjsonValue->AddMember(key, value, *allocator);
}

void JSON_value::AddFloat2(const char * name, float2 value)
{
	std::string myname(name);
	rapidjson::Value key(myname.c_str(), myname.size(), *allocator);

	rapidjson::Value float2(rapidjson::kArrayType);
	float2.PushBack(value.x, *allocator);
	float2.PushBack(value.y, *allocator);

	rapidjsonValue->AddMember(key, float2, *allocator);
}

void JSON_value::AddFloat3(const char * name, float3 value)
{
	std::string myname(name);
	rapidjson::Value key(myname.c_str(), myname.size(), *allocator);

	rapidjson::Value float3(rapidjson::kArrayType);
	float3.PushBack(value.x, *allocator);
	float3.PushBack(value.y, *allocator);
	float3.PushBack(value.z, *allocator);

	rapidjsonValue->AddMember(key, float3, *allocator);
}

void JSON_value::AddFloat4(const char * name, float4 value)
{
	std::string myname(name);
	rapidjson::Value key(myname.c_str(), myname.size(), *allocator);

	rapidjson::Value float4(rapidjson::kArrayType);
	float4.PushBack(value.x, *allocator);
	float4.PushBack(value.y, *allocator);
	float4.PushBack(value.z, *allocator);
	float4.PushBack(value.w, *allocator);

	rapidjsonValue->AddMember(key, float4, *allocator);
}

void JSON_value::AddFloat4x4(const char * name, float4x4 value)
{
	std::string myname(name);
	rapidjson::Value key(myname.c_str(), myname.size(), *allocator);

	rapidjson::Value mat(rapidjson::kArrayType);
	for (unsigned i = 0; i < 4; i++)
	{
		for (unsigned j = 0; j < 4; j++)
		{
			mat.PushBack(value[i][j], *allocator);
		}
	}

	rapidjsonValue->AddMember(key, mat, *allocator);
}

void JSON_value::AddQuat(const char * name, Quat value)
{
	std::string myname(name);
	rapidjson::Value key(myname.c_str(), myname.size(), *allocator);

	rapidjson::Value quat(rapidjson::kArrayType);
	quat.PushBack(value.x, *allocator);
	quat.PushBack(value.y, *allocator);
	quat.PushBack(value.z, *allocator);
	quat.PushBack(value.w, *allocator);

	rapidjsonValue->AddMember(key, quat, *allocator);
}

void JSON_value::AddString(const char * name, const char* value)
{
	std::string myname(name);
	rapidjson::Value key(myname.c_str(), myname.size(), *allocator);

	std::string myvalue(value);
	rapidjson::Value rvalue(myvalue.c_str(), myvalue.size(), *allocator);

	rapidjsonValue->AddMember(key, rvalue, *allocator);
}

void JSON_value::AddValue(const char* name, const JSON_value &value)
{
	switch (this->rapidjsonValue->GetType())
	{
	case rapidjson::kObjectType:
	{
		std::string myname(name);
		rapidjson::Value key(myname.c_str(), myname.size(), *allocator);
		this->rapidjsonValue->AddMember(key, *value.rapidjsonValue, *allocator);
	}
	break;
	case rapidjson::kArrayType:
		this->rapidjsonValue->PushBack(*value.rapidjsonValue, *allocator);
		break;
	}
}

int JSON_value::GetInt(const char * name) const
{
	rapidjson::Value::ConstMemberIterator itr = rapidjsonValue->FindMember(name);
	if (itr != rapidjsonValue->MemberEnd())
	{
		return itr->value.GetInt();
	}
	else
	{
		LOG("Member %s not found!", name);
		return 0;
	}
}

unsigned JSON_value::GetUint(const char * name, unsigned defaultValue) const
{
	rapidjson::Value::ConstMemberIterator itr = rapidjsonValue->FindMember(name);
	if (itr != rapidjsonValue->MemberEnd())
	{
		return itr->value.GetUint();
	}
	else
	{
		LOG("Member %s not found!", name);
		return defaultValue;
	}
}

float JSON_value::GetFloat(const char * name) const
{
	rapidjson::Value::ConstMemberIterator itr = rapidjsonValue->FindMember(name);
	if (itr != rapidjsonValue->MemberEnd())
	{
		return itr->value.GetFloat();
	}
	else
	{
		LOG("Member %s not found!", name);
		return 0.f;
	}
}

float2 JSON_value::GetFloat2(const char * name) const
{
	rapidjson::Value::ConstMemberIterator itr = rapidjsonValue->FindMember(name);
	if (itr != rapidjsonValue->MemberEnd())
	{
		float2 ret(itr->value[0].GetFloat(), itr->value[1].GetFloat());
		return ret;
	}
	else
	{
		LOG("Member %s not found!", name);
		return float2::zero;
	}
}

float3 JSON_value::GetFloat3(const char * name) const
{
	rapidjson::Value::ConstMemberIterator itr = rapidjsonValue->FindMember(name);
	if (itr != rapidjsonValue->MemberEnd())
	{
		float3 ret(itr->value[0].GetFloat(), itr->value[1].GetFloat(), itr->value[2].GetFloat());
		return ret;
	}
	else
	{
		LOG("Member %s not found!", name);
		return float3::zero;
	}
}

float3 JSON_value::GetColor3(const char * name) const
{
	rapidjson::Value::ConstMemberIterator itr = rapidjsonValue->FindMember(name);
	if (itr != rapidjsonValue->MemberEnd())
	{
		float3 ret(itr->value[0].GetFloat(), itr->value[1].GetFloat(), itr->value[2].GetFloat());
		return ret;
	}
	else
	{
		LOG("Member %s not found!", name);
		return float3::one;
	}
}

float4 JSON_value::GetFloat4(const char * name) const
{
	rapidjson::Value::ConstMemberIterator itr = rapidjsonValue->FindMember(name);
	if (itr != rapidjsonValue->MemberEnd())
	{
		float4 ret(itr->value[0].GetFloat(), itr->value[1].GetFloat(),
			itr->value[2].GetFloat(), itr->value[3].GetFloat());
		return ret;
	}
	else
	{
		LOG("Member %s not found!", name);
		return float4::zero;
	}
}

float4 JSON_value::GetColor4(const char * name) const
{
	rapidjson::Value::ConstMemberIterator itr = rapidjsonValue->FindMember(name);
	if (itr != rapidjsonValue->MemberEnd())
	{
		float4 ret(itr->value[0].GetFloat(), itr->value[1].GetFloat(),
			itr->value[2].GetFloat(), itr->value[3].GetFloat());
		return ret;
	}
	else
	{
		LOG("Member %s not found!", name);
		return float4::one;
	}
}

float4x4 JSON_value::GetFloat4x4(const char * name) const
{
	rapidjson::Value::ConstMemberIterator itr = rapidjsonValue->FindMember(name);
	if (itr != rapidjsonValue->MemberEnd())
	{
		float4x4 mat;
		for (unsigned i = 0; i < 4; i++)
		{
			for (unsigned j = 0; j < 4; j++)
			{
				mat.v[i][j] = itr->value[4*i+j].GetFloat();
			}
		}

		return mat;
	}
	else
	{
		LOG("Member %s not found!", name);
		return float4x4::identity;
	}
}

Quat JSON_value::GetQuat(const char * name) const
{
	rapidjson::Value::ConstMemberIterator itr = rapidjsonValue->FindMember(name);
	if (itr != rapidjsonValue->MemberEnd())
	{
		Quat ret(itr->value[0].GetFloat(), itr->value[1].GetFloat(),
			itr->value[2].GetFloat(), itr->value[3].GetFloat());
		return ret;
	}
	else
	{
		LOG("Member %s not found!", name);
		return Quat::identity;
	}
}

const char* JSON_value::GetString(const char * name) const
{
	rapidjson::Value::ConstMemberIterator itr = rapidjsonValue->FindMember(name);
	if (itr != rapidjsonValue->MemberEnd())
	{
		return itr->value.GetString();
	}
	else
	{
		LOG("Member %s not found!", name);
		return nullptr;
	}
}

JSON_value* JSON_value::GetValue(unsigned index)
{
	JSON_value * val = new JSON_value(allocator);
	val->rapidjsonValue->CopyFrom(rapidjsonValue->operator[](index), *allocator);
	valuesAllocated.push_back(val);
	return val;
}

JSON_value * JSON_value::GetValue(const char* name)
{
	if (rapidjsonValue->HasMember(name))
	{
		JSON_value * val = new JSON_value(allocator);
		val->rapidjsonValue->CopyFrom(rapidjsonValue->operator[](name), *allocator);
		valuesAllocated.push_back(val);
		return val;
	}
	return nullptr;
}

unsigned JSON_value::Size() const
{
	bool isArray = this->rapidjsonValue->IsArray();
	assert(isArray);
	if (!isArray)
	{
		LOG("This value is not an array!");
		return 0;
	}
	return this->rapidjsonValue->Size();
}


JSON::JSON()
{
	document = new rapidjson::Document();
	document->SetObject();
	allocator = &document->GetAllocator();
	jsonBuffer = new rapidjson::StringBuffer();
}

JSON::JSON(const char *data)
{
	document = new rapidjson::Document();
	document->SetObject();
	document->Parse(data);
	allocator = &document->GetAllocator();
	jsonBuffer = new rapidjson::StringBuffer();
}

JSON::~JSON()
{
	for (auto &value : valuesAllocated)
	{
		RELEASE(value);
	}
	RELEASE(document);
	RELEASE(jsonBuffer);
}


JSON_value * JSON::CreateValue(rapidjson::Type type)
{
	JSON_value* ret = new JSON_value(allocator, type);
	valuesAllocated.push_back(ret);
	return ret;
}

void JSON::AddValue(const char* name, const JSON_value& value)
{
	std::string myname(name);
	rapidjson::Value key(myname.c_str(), myname.size(), *allocator);
	document->AddMember(key, *value.rapidjsonValue, *allocator);
}

JSON_value * JSON::GetValue(const char* name)
{
	if (document->HasMember(name))
	{
		JSON_value * val = new JSON_value(allocator);
		val->rapidjsonValue->CopyFrom(document->operator[](name), *allocator);
		valuesAllocated.push_back(val);
		return val;
	}
	return nullptr;
}


std::string JSON::ToString() const
{
	jsonBuffer->Clear();
	rapidjson::Writer<rapidjson::StringBuffer> writer(*jsonBuffer);
	document->Accept(writer);

	return jsonBuffer->GetString();
}

unsigned JSON::Size()
{
	jsonBuffer->Clear();
	rapidjson::Writer<rapidjson::StringBuffer> writer(*jsonBuffer);
	document->Accept(writer);
	return jsonBuffer->GetSize();
}



