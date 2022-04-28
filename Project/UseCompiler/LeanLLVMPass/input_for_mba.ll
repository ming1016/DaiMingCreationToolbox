; ModuleID = 'inputs/input_for_mba.c'
source_filename = "inputs/input_for_mba.c"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx12.0.0"

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone ssp uwtable willreturn
define signext i8 @foo(i8 signext %0, i8 signext %1, i8 signext %2, i8 signext %3) local_unnamed_addr #0 {
  %5 = add i8 %1, %0
  %6 = add i8 %5, %2
  %7 = add i8 %6, %3
  ret i8 %7
}

; Function Attrs: mustprogress nofree nounwind readonly ssp uwtable willreturn
define i32 @main(i32 %0, i8** nocapture readonly %1) local_unnamed_addr #1 {
  %3 = getelementptr inbounds i8*, i8** %1, i64 1
  %4 = load i8*, i8** %3, align 8, !tbaa !5
  %5 = call i32 @atoi(i8* %4)
  %6 = getelementptr inbounds i8*, i8** %1, i64 2
  %7 = load i8*, i8** %6, align 8, !tbaa !5
  %8 = call i32 @atoi(i8* %7)
  %9 = getelementptr inbounds i8*, i8** %1, i64 3
  %10 = load i8*, i8** %9, align 8, !tbaa !5
  %11 = call i32 @atoi(i8* %10)
  %12 = getelementptr inbounds i8*, i8** %1, i64 4
  %13 = load i8*, i8** %12, align 8, !tbaa !5
  %14 = call i32 @atoi(i8* %13)
  %15 = add i32 %8, %5
  %16 = add i32 %15, %11
  %17 = add i32 %16, %14
  %18 = shl i32 %17, 24
  %19 = ashr exact i32 %18, 24
  ret i32 %19
}

; Function Attrs: mustprogress nofree nounwind readonly willreturn
declare i32 @atoi(i8* nocapture) local_unnamed_addr #2

attributes #0 = { mustprogress nofree norecurse nosync nounwind readnone ssp uwtable willreturn "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nofree nounwind readonly ssp uwtable willreturn "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nofree nounwind readonly willreturn "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 7, !"frame-pointer", i32 2}
!4 = !{!"Homebrew clang version 13.0.1"}
!5 = !{!6, !6, i64 0}
!6 = !{!"any pointer", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
