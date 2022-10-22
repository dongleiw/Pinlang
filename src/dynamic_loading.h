#pragma once

#include "define.h"
#include <map>
#include <string>
#include <vector>

class AstNodeBlockStmt;
class AstNodeComplexFnDef;

/*
 * 动态加载动态库
 * 提供三个内置函数:
 *		1. dl_open(path str) int
 *		2. dl_getFn[T Fn](handler int, fn_name str) T
 *		3. dl_close(handler int);
 *  每个加载的动态库有唯一id
 *
 *  与C shared library的类型的对应关系
 *		参数i32: 对应C中的4byte有符号整数, 比如int
 *		参数u32: 对应C中的4byte无符号整数, 比如uint32_t
 *		参数i64: 对应C中的8byte有符号整数, 比如ssize_t
 *		参数u64: 对应C中的8byte无符号整数, 比如size_t
 *		参数str:
 *			对应C中的const char*. 调用函数时, 将str的数据指针传入
 *		返回值str:
 *			对应C中, 返回const char*
 *			函数返回后, 生成str
 *
 */
class DynamicLoading {
public:
	typedef void* DlHandler;
	struct DynLib {
		std::string path;
		DlHandler	handler;
		int			dynlib_instance_id;

		std::map<std::string, void*> fn_list; // 保存从动态库中获取的符号信息
	};

public:
	DynamicLoading();

	static void RegisterFn(AstNodeBlockStmt& block_stmt);

	// 加载动态库
	bool OpenSharedLibrary(std::string path, int& dynlib_instance_id, std::string& err);
	// 从加载的动态库中获取某个函数. 符号
	bool GetFn(int dynlib_instance_id, std::string fn_name, void** dynlib_fn, std::string& err);
	void CloseSharedLibrary(int id);

	bool FnExist(int dynlib_instance_id, void* fn) const;

	bool SupportType(TypeId tid) const;

	BuiltinFnCallback GetBuiltinFnCallback(std::vector<TypeId> params_tid, TypeId return_tid) const;

private:
	static AstNodeComplexFnDef* create_astnode_fn_dl_open();
	static AstNodeComplexFnDef* create_astnode_fn_dl_getFn();

private:
	std::vector<DynLib> m_dynlib_instances;
};
