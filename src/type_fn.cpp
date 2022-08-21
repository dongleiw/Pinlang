#include "type_fn.h"
#include "log.h"
#include "type.h"
#include "type_mgr.h"
#include <system_error>

TypeInfoFn::TypeInfoFn(std::vector<Parameter> params, TypeId return_tid) {
	m_params	 = params;
	m_return_tid = return_tid;
	set_name();
	set_uniq_fn_name_suffix();
	m_typegroup_id = TYPE_GROUP_ID_FUNCTION;
}

std::string TypeInfoFn::GetUniqFnNameSuffix(std::vector<TypeId> args_tid){
	std::string s = "";
	for (size_t i = 0; i < args_tid.size(); i++) {
		std::string arg_name = GET_TYPENAME(args_tid[i]);
		s+= arg_name;
		if (i + 1 != args_tid.size()) {
			s+= "_";
		}
	}
	return s;
}
void TypeInfoFn::set_uniq_fn_name_suffix() {
	std::vector<TypeId> args_tid;
	for (size_t i = 0; i < m_params.size(); i++) {
		args_tid.push_back(m_params.at(i).arg_tid);
	}
	m_uniq_fn_name_suffix = GetUniqFnNameSuffix(args_tid);
}
void TypeInfoFn::set_name() {
	m_name = "fn(";
	for (size_t i = 0; i < m_params.size(); i++) {
		std::string arg_name = GET_TYPENAME(m_params[i].arg_tid);
		m_name += arg_name;
		if (i + 1 != m_params.size()) {
			m_name += ", ";
		}
	}
	m_name += ")" + GET_TYPENAME(m_return_tid);
}
bool TypeInfoFn::VerifyArgsType(std::vector<TypeId> args_type) {
	if (args_type.size() != m_params.size()) {
		return false;
	}

	for (size_t i = 0; i < m_params.size(); i++) {
		TypeId expect_tid = m_params.at(i).arg_tid;
		TypeId give_tid	  = args_type.at(i);
		if (expect_tid != give_tid) {
			log_error("type of arg %lu not match: expect[%s] give[%s]", i, GET_TYPENAME_C(expect_tid), GET_TYPENAME_C(give_tid));
			return false;
		}
	}
	return true;
}
