#include "astnode.h"
#include "astnode_complex_fndef.h"
#include "astnode_for.h"
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
