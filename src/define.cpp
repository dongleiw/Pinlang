#include "define.h"

#include "astnode_type.h"

ParserParameter ParserParameter::DeepClone() {
	ParserParameter newone;

	newone.name = this->name;
	newone.type = this->type->DeepCloneT();

	return newone;
}
ParserClassField ParserClassField::DeepClone() {
	ParserClassField newone;

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
bool is_vec_typeid_equal(const std::vector<TypeId>& a, const std::vector<TypeId>& b){
	if (a.size() != b.size())
		return false;
	for (size_t i = 0; i < a.size(); i++) {
		if (a.at(i) != b.at(i)) {
			return false;
		}
	}
	return true;
}
ParserInitElement ParserInitElement::DeepClone(){
	ParserInitElement newone;
	
	newone.attr_name=attr_name;
	newone.attr_value = attr_value->DeepClone();

	return newone;
}
