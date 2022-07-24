#include "verify_context.h"

#include <assert.h>


void VerifyContext::PushStack(){
	Stack* new_stack = new Stack(m_top_stack);
	m_top_stack = new_stack;
}
void VerifyContext::PopSTack(){
	assert(m_top_stack!=NULL);
	m_top_stack = m_top_stack->GetNextStack();
}
