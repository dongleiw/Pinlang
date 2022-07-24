#include "execute_context.h"

#include <assert.h>

ExecuteContext::ExecuteContext() : m_top_stack(NULL) {
}
void ExecuteContext::PushStack() {
	Stack* new_stack = new Stack(m_top_stack);
	new_stack->EnterBlock(&m_global_vt);
	m_top_stack = new_stack;
}
void ExecuteContext::PopSTack() {
	assert(m_top_stack != NULL);
	m_top_stack = m_top_stack->GetNextStack();
}
