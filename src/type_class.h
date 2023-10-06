#pragma once

#include "type.h"
#include <llvm-12/llvm/IR/DerivedTypes.h>

/*
 */
class TypeInfoClass : public TypeInfo {
public:
	struct Field {
		std::string name;
		TypeId		tid;
		int			mem_offset; // 该字段的内存地址偏移
	};

public:
	TypeInfoClass(std::string class_name);
	void				InitBuiltinMethods(VerifyContext& ctx) override;
	virtual llvm::Type* GetLLVMIRType(CompileContext& cctx) override;
	virtual void		ConstructFields(CompileContext& cctx, llvm::Value* obj) override;
	virtual void		ConstructDefault(CompileContext& cctx, llvm::Value* obj) override;

	void AddDefaultConstructor(VerifyContext& ctx);

	std::vector<Field> GetField() const { return m_field_list; }
	size_t			   GetFieldIndex(std::string field_name) const;
	TypeId			   GetFieldType(std::string field_name) const;
	void			   SetFields(std::vector<std::pair<std::string, TypeId>> fields);
	bool			   HasField(std::string field_name) const;

	void SetDefaultConstrutorFnId(std::string fnid) { m_default_constructor_fnid = fnid; }

private:
	std::vector<Field> m_field_list;

	llvm::StructType* m_ir_type;
	std::string		  m_default_constructor_fnid; // 缺省构造函数的fnid. 如果不存在则为空
};
