; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() !dbg !4 {
entry:
  %i = alloca i64, align 8
  %s = alloca i64, align 8
  store i64 0, ptr %s, align 4, !dbg !11
  call void @llvm.dbg.declare(metadata ptr %s, metadata !9, metadata !DIExpression()), !dbg !12
  br label %for.init, !dbg !11

for.init:                                         ; preds = %entry
  store i64 0, ptr %i, align 4, !dbg !11
  call void @llvm.dbg.declare(metadata ptr %i, metadata !10, metadata !DIExpression()), !dbg !13
  br label %for.cond, !dbg !11

for.cond:                                         ; preds = %for.update, %for.init
  %i1 = load i64, ptr %i, align 4, !dbg !11
  %icmpsletmp = icmp sle i64 %i1, 2, !dbg !11
  br i1 %icmpsletmp, label %for.body, label %for.exit, !dbg !11

for.body:                                         ; preds = %for.cond
  %s2 = load i64, ptr %s, align 4, !dbg !11
  %i3 = load i64, ptr %i, align 4, !dbg !11
  %addtmp = add i64 %s2, %i3, !dbg !11
  store i64 %addtmp, ptr %s, align 4, !dbg !11
  br label %for.update, !dbg !11

for.update:                                       ; preds = %for.body
  %i4 = load i64, ptr %i, align 4, !dbg !11
  %addtmp5 = add i64 %i4, 1, !dbg !11
  store i64 %addtmp5, ptr %i, align 4, !dbg !11
  br label %for.cond, !dbg !11

for.exit:                                         ; preds = %for.cond
  %s6 = load i64, ptr %s, align 4, !dbg !11
  ret i64 %s6, !dbg !11
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "range_only.vyn.ll", directory: "/home/rick/Projects/Vyn/test/vec_for")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 2, type: !5, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !10}
!9 = !DILocalVariable(name: "s", scope: !4, file: !1, line: 3, type: !7)
!10 = !DILocalVariable(name: "i", scope: !4, file: !1, line: 4, type: !7)
!11 = !DILocation(line: 2, column: 1, scope: !4)
!12 = !DILocation(line: 3, column: 1, scope: !4)
!13 = !DILocation(line: 4, column: 10, scope: !4)
