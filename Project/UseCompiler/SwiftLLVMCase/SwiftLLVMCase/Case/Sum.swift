//
//  Sum.swift
//  SwiftLLVMCase
//
//  Created by Ming Dai on 2022/4/19.
//

import Foundation
import llvm

func sum() {
    let m = Module(name: "Sum")
    let bd = IRBuilder(module: m) // IRBuilder 是一个光标，在上下文中移动
    let main = bd.addFunction("main", type: FunctionType([], IntType.int64))
    let entry = main.appendBasicBlock(named: "entry")
    bd.positionAtEnd(of: entry)
    
    let i1 = IntType.int64.constant(51)
    let sum = bd.buildAdd(i1, i1)
    bd.buildRet(sum)
    
    m.dump()
    
    // 打印结果
    /*
     ; ModuleID = 'Sum'
     source_filename = "Sum"

     define i64 @main() {
     entry:
       ret i64 102
     }
     */
    
}

/*
 这段 c 在 IR 中实现
 int sum(int a, int b) {
   return a + b;
 }
 */
func cSum() {
    let m = Module(name: "CSum")
    let bd = IRBuilder(module: m)
    let f1 = bd.addFunction("sum", type: FunctionType([IntType.int32, IntType.int32], IntType.int32))
    
    // 添加基本块
    let entryBB = f1.appendBasicBlock(named: "entry")
    bd.positionAtEnd(of: entryBB)
    
    let a = f1.parameters[0]
    let b = f1.parameters[1]
    
    let tmp = bd.buildAdd(a, b)
    bd.buildRet(tmp)
    
    m.dump()
    
}
