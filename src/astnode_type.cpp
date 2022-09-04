#include "astnode_type.h"
#include "define.h"
#include "function.h"
#include "log.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"
#include "verify_context.h"
#include <memory>
#include <vector>

void AstNodeType::InitWithType() {
	m_type_kind = TYPE_KIND_TYPE;
}
void AstNodeType::InitWithIdentifier(std::string id) {
	m_type_kind = TYPE_KIND_IDENTIFIER;
	m_id		= id;
}

/*
 * 调用函数
 */
VerifyContextResult AstNodeType::Verify(VerifyContext& ctx) {
	log_debug("verify type: type_kind[%d] id[%s]", m_type_kind, m_id.c_str());
	VerifyContextResult vr;
	switch (m_type_kind) {
	case TYPE_KIND_TYPE:
		vr.SetResultTypeId(TYPE_ID_TYPE);
		break;
	case TYPE_KIND_IDENTIFIER:
		vr.SetResultTypeId(ctx.GetCurStack()->GetVariableType(m_id));
		break;
	default:
		panicf("unknown type_kind[%d]", m_type_kind);
		break;
	}
	return vr;
}
std::map<std::string, TypeId> AstNodeType::InferType(TypeId target_tid) const {
	std::map<std::string,TypeId> result;
	switch (m_type_kind) {
	case TYPE_KIND_TYPE:
		if(target_tid!=TYPE_ID_TYPE){
			panicf("bug");
		}
		return result;
		break;
	case TYPE_KIND_IDENTIFIER:
		result[m_id] = target_tid;
		return result;
		break;
	default:
		panicf("unknown type_kind[%d]", m_type_kind);
		break;
	}
}
