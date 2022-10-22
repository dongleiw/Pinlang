#pragma once

#include <map>
#include <string>
#include <utility>

#include "define.h"

class Function;
class VerifyContext;
class AstNodeComplexFnDef;

/*
 * 类型信息
 */
class TypeInfo {
public:
	// 类型的方法
	struct Method {
		std::string			 method_name;
		AstNodeComplexFnDef* method_node;
	};
	// 类型的方法的一个具体实例
	struct MethodInstance {
		std::string method_name; // 方法名
		Function*	fn;			 // 方法的实现
	};
	struct Constraint {
		TypeId						constraint_tid;
		std::vector<Method>			methods;
		std::vector<MethodInstance> concrete_methods;
		MethodIndex					AddConcreteMethod(std::string method_name, Function* fn);
	};
	struct Field {
		std::string name;
		TypeId		tid;
		int			mem_offset; // 该字段的内存地址偏移
	};

public:
	TypeInfo() : m_typeid(TYPE_ID_NONE), m_typegroup_id(TYPE_GROUP_ID_UNRESOLVE) {
	}

	TypeId		GetTypeId() const { return m_typeid; }
	void		SetTypeId(TypeId tid) { m_typeid = tid; }
	std::string GetName() { return m_name; }
	void		SetName(std::string name) { m_name = name; }

	/*
	 * 根据方法名和参数类型查找函数. 如果需要则实例化
	 */
	MethodIndex GetConcreteMethod(VerifyContext& ctx, std::string method_name, std::vector<TypeId> args_tid, TypeId return_tid);
	/*
	 * 根据方法名和函数类型查找函数. 如果需要则实例化
	 */
	MethodIndex GetConcreteMethod(VerifyContext& ctx, std::string method_name, TypeId tid);
	/*
	 * 仅仅根据方法名查找函数. 如果需要则实例化
	 */
	MethodIndex GetConcreteMethod(VerifyContext& ctx, std::string method_name);
	/*
	 * 只根据函数名查找. 找到多个会panic
	 */
	MethodIndex GetMethodIdx(std::string method_name) const;

	Function* GetMethodByIdx(MethodIndex method_idx);

	virtual void InitBuiltinMethods(VerifyContext& ctx) {}

	void SetTypeGroupId(TypeGroupId tgid) { m_typegroup_id = tgid; }

	bool IsFn() const { return m_typegroup_id == TYPE_GROUP_ID_FUNCTION; }
	bool IsArray() const { return m_typegroup_id == TYPE_GROUP_ID_ARRAY; }
	bool IsType() const { return m_typeid == TYPE_ID_TYPE; }
	bool IsConstraint() const { return m_typegroup_id == TYPE_GROUP_ID_CONSTRAINT; }
	bool IsGenericType() const { return m_typegroup_id == TYPE_GROUP_ID_VIRTUAL_GTYPE; }
	bool IsPrimaryType() const { return m_typegroup_id == TYPE_GROUP_ID_PRIMARY; }
	bool IsClass() const { return m_typegroup_id == TYPE_GROUP_ID_CLASS; }
	bool IsTuple() const { return m_typegroup_id == TYPE_GROUP_ID_TUPLE; }

	void AddConstraint(TypeId constraint_tid, std::vector<AstNodeComplexFnDef*> methods);
	bool MatchConstraint(TypeId tid) const;
	/*
	 * 根据constraint的名字 + 方法名字 + 方法参数信息搜索方法. 返回所有匹配的函数位置信息
	 */
	std::vector<MethodIndex> GetConstraintMethod(VerifyContext& ctx, std::string constraint_name, std::string method_name, std::vector<TypeId> method_params_tid);

	bool HasField(std::string field_name) const;
	//void			   AddField(std::string field_name, TypeId tid);
	void			   SetFields(std::vector<std::pair<std::string, TypeId>> fields);
	TypeId			   GetFieldType(std::string field_name) const;
	std::vector<Field> GetField() const { return m_field_list; }

	int GetMemSize() const { return m_mem_size; }
	int GetMemAlignSize() const { return m_mem_align_size; }

protected:
	/*
	 * 对齐字段, 计算每个字段的偏移
	 * 对齐的需求:
	 *		1byte: i8 u8
	 *		2byte: i16 u16
	 *		4byte: i32 u32
	 *		8byte: i64 u64
	 *
	 *		组合类型的对齐需求为"字段的最大对齐需求"
	 */
	void align_field();

protected:
	TypeId		m_typeid;
	TypeGroupId m_typegroup_id;
	std::string m_name;
	int			m_mem_size;		  // 内存大小
	int			m_mem_align_size; // 内存对齐大小

	/*
	 * 该类型实现的约束列表. 
	 */
	std::vector<Constraint> m_constraints;

	std::vector<Field> m_field_list;
};
