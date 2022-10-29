#include "verify_context.h"
#include "dynamic_loading.h"
#include "type.h"

#include <assert.h>

VerifyContext::VerifyContext(AstNodeBlockStmt* global_block) {
	m_global_vt.InitAsGlobal();
	m_global_block = global_block;
}

void VerifyContext::PushStack() {
	Stack* new_stack = new Stack(m_top_stack);
	new_stack->EnterBlock(&m_global_vt);
	m_top_stack = new_stack;
}
void VerifyContext::PopSTack() {
	assert(m_top_stack != nullptr);
	m_top_stack = m_top_stack->GetNextStack();
}
