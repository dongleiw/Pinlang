#include "execute_context.h"
#include "type.h"
#include "variable.h"

#include <assert.h>

ExecuteContext::ExecuteContext() : m_top_stack(nullptr) {
	m_global_vt.InitAsGlobal();
}
void ExecuteContext::PushStack() {
	Stack* new_stack = new Stack(m_top_stack);
	new_stack->EnterBlock(&m_global_vt);
	m_top_stack = new_stack;
}
void ExecuteContext::PopStack() {
	assert(m_top_stack != nullptr);
	m_top_stack = m_top_stack->GetNextStack();
}
