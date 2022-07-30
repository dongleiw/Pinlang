#include "type.h"
#include "log.h"


void TypeInfo::AddMethod(std::string method_name, Function* function){
	m_methods[method_name] = function;
}
Function* TypeInfo::GetMethodOrNilByName(std::string method_name){
	auto iter = m_methods.find(method_name);
	if(iter==m_methods.end()){
		return nullptr;
	}else{
		return iter->second;
	}
}
Function* TypeInfo::GetMethodByName(std::string method_name){
	auto iter = m_methods.find(method_name);
	if(iter==m_methods.end()){
		panicf("type[%d:%s] doesn't have method[%s]", GetTypeId(), GetName().c_str(), method_name.c_str());
		return nullptr;
	}else{
		return iter->second;
	}
}
