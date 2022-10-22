#include "verify_context.h"
#include "type.h"

#include <assert.h>

VerifyContext::VerifyContext(){
	m_global_vt.InitAsGlobal();
}

void VerifyContext::PushStack(){
	Stack* new_stack = new Stack(m_top_stack);
	new_stack->EnterBlock(&m_global_vt);
	m_top_stack = new_stack;
}
void VerifyContext::PopSTack(){
	assert(m_top_stack!=nullptr);
	m_top_stack = m_top_stack->GetNextStack();
}
