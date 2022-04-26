/*

静态调用，通过函数指针调用的不考虑。

使用方式：
opt -load-pass-plugin libStaticCallCounter.dylib -passes="print<static-cc>" -disable-output <input-llvm-file>

*/



