//
//  ControlFlow.swift
//  SwiftLLVMCase
//
//  Created by Ming Dai on 2022/4/20.
//

import Foundation
import llvm

/*
 这段 Swift 代码，通过在 IR 中构建
 func giveMeNumber(_ isBig : Bool) -> Int {
     let re : Int
     if !isBig {
         // the fibonacci series (sort of)
         re = 3
     } else {
         // the fibonacci series (sort of) backwards
         re = 4
     }
     return re
 }
 */

func controlFlow() {
    let m = Module(name: "CF")
    let bd = IRBuilder(module: m)
    let f1 = bd.addFunction("giveMeNumber", type: FunctionType([IntType.int1], FloatType.double))
    let entryBB = f1.appendBasicBlock(named: "entry")
    bd.positionAtEnd(of: entryBB)
    
    // 给本地变量分配空间 let retVal : Double
    let local = bd.buildAlloca(type: IntType.int32, name: "local")
    
    // 条件比较 if !backward
    let test = bd.buildICmp(f1.parameters[0], IntType.int1.zero(), .equal)
    
    // 创建 block
    let thenBB = f1.appendBasicBlock(named: "then")
    let elseBB = f1.appendBasicBlock(named: "else")
    let mergeBB = f1.appendBasicBlock(named: "merge")
    
    bd.buildCondBr(condition: test, then: thenBB, else: elseBB)
    
    // 指到 then block
    bd.positionAtEnd(of: thenBB)
    let thenVal = IntType.int32.constant(3)
    bd.buildBr(mergeBB) // 到 merge block
    
    // 指到 else block
    bd.positionAtEnd(of: elseBB)
    let elseVal = IntType.int32.constant(4)
    bd.buildBr(mergeBB) // 到 merge block
    
    // 指到 merge block
    bd.positionAtEnd(of: mergeBB)
    let phi = bd.buildPhi(IntType.int32, name: "phi_example")
    phi.addIncoming([
        (thenVal, thenBB),
        (elseVal, elseBB)
    ])
    // 赋值给本地变量
    bd.buildStore(phi, to: local)
    let ret = bd.buildLoad(local, type: IntType.int32, name: "ret")
    bd.buildRet(ret)
    
    m.dump()
    
    /*
     ; ModuleID = 'CF'
     source_filename = "CF"

     define double @calculateFibs(i1 %0) {
     entry:
       %local = alloca double, align 8
       %1 = icmp eq i1 %0, false
       br i1 %1, label %then, label %else

     then:                                             ; preds = %entry
       br label %merge

     else:                                             ; preds = %entry
       br label %merge

     merge:                                            ; preds = %else, %then
       %phi_example = phi double [ 0x3F8702E05C0B8170, %then ], [ 0x3F82C9FB4D812CA0, %else ]
       store double %phi_example, double* %local, align 8
       %ret = load double, double* %local, align 8
       ret double %ret
     }
     */
    
}
























