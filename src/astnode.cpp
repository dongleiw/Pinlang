#include "astnode.h"
#include "astnode_complex_fndef.h"
#include "astnode_for.h"
#include "compile_context.h"
#include "define.h"
#include "instruction.h"
#include "log.h"

bool AstNode::IsInFor() const {
	AstNode* cur = m_parent;
	while (cur != nullptr) {
		if (nullptr != dynamic_cast<AstNodeFor*>(cur)) {
			return true;
		}
		if (nullptr != dynamic_cast<AstNodeComplexFnDef*>(cur)) {
			return false;
		}
		cur = cur->m_parent;
	}
	return false;
}
bool AstNode::IsInFn() const {
	AstNode* cur = m_parent;
	while (cur != nullptr) {
		if (nullptr != dynamic_cast<AstNodeComplexFnDef*>(cur)) {
			return true;
		}
		cur = cur->m_parent;
	}
	return false;
}
void AstNode::verify_begin() {
	if (m_verify_status != NOT_VERIFIED) {
		panicf("wrong verify status[%d]", m_verify_status);
	} else {
		m_verify_status = IN_VERIFY;
	}
}
void AstNode::verify_end() {
	if (m_verify_status != IN_VERIFY) {
		panicf("wrong verify status[%d]", m_verify_status);
	} else {
		m_verify_status = VERIFIED;
	}
}
CompileResult AstNode::Compile(CompileContext& cctx) {
	panicf("not implemented");
}
Variable* AstNode::Execute(ExecuteContext& ctx) {
	panicf("not implemented");
}
void AstNode::Copy(AstNode& from) {
	m_result_typeid = from.m_result_typeid;
	//m_parent				= from.m_parent;
	//m_verify_status			= from.m_verify_status;
	m_is_exit_node			= from.m_is_exit_node;
	m_compile_to_left_value = from.m_compile_to_left_value;
}
void AstNode::ClearVerifyState() {
	m_result_typeid			= TYPE_ID_NONE;
	m_verify_status			= NOT_VERIFIED;
	m_compile_to_left_value = false;
}
