#include "execute_context.h"
#include "type.h"
#include "variable.h"

#include <assert.h>

ExecuteContext::ExecuteContext() : m_top_stack(NULL) {
	//init_global_vt();
	m_global_vt.InitAsGlobal();
}
void ExecuteContext::PushStack() {
	Stack* new_stack = new Stack(m_top_stack);
	new_stack->EnterBlock(&m_global_vt);
	m_top_stack = new_stack;
}
void ExecuteContext::PopStack() {
	assert(m_top_stack != NULL);
	m_top_stack = m_top_stack->GetNextStack();
}
void ExecuteContext::init_global_vt(){
	m_global_vt.AddVariable("type", Variable::CreateTypeVariable(TYPE_ID_TYPE));
	m_global_vt.AddVariable("int", Variable::CreateTypeVariable(TYPE_ID_INT));
}
