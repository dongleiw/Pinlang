#include "define.h"

#include "astnode_type.h"

ParserParameter ParserParameter::DeepClone() {
	ParserParameter newone;

	newone.name = this->name;
	newone.type = this->type->DeepCloneT();

	return newone;
}
ParserGenericParam ParserGenericParam::DeepClone() {
	ParserGenericParam newone;

	newone.type_name	   = this->type_name;
	newone.constraint_name = this->constraint_name;
	for (auto iter : this->constraint_generic_params) {
		newone.constraint_generic_params.push_back(iter->DeepCloneT());
	}
	return newone;
}
ParserFnDeclare ParserFnDeclare::DeepClone(){
	ParserFnDeclare newone;

	newone.fnname = this->fnname;
	for(auto iter:this->param_list){
		newone.param_list.push_back(iter.DeepClone());
	}
	if(return_type!=nullptr)
		newone.return_type = return_type->DeepCloneT();

	return newone;
}
