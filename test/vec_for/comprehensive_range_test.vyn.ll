; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() !dbg !4 {
entry:
  %i69 = alloca i64, align 8
  %sum5 = alloca i64, align 8
  %i49 = alloca i64, align 8
  %sum4 = alloca i64, align 8
  %i29 = alloca i64, align 8
  %sum3 = alloca i64, align 8
  %i12 = alloca i64, align 8
  %sum2 = alloca i64, align 8
  %i = alloca i64, align 8
  %sum1 = alloca i64, align 8
  store i64 0, ptr %sum1, align 4, !dbg !19
  call void @llvm.dbg.declare(metadata ptr %sum1, metadata !9, metadata !DIExpression()), !dbg !20
  br label %for.init, !dbg !19

for.init:                                         ; preds = %entry
  store i64 0, ptr %i, align 4, !dbg !19
  call void @llvm.dbg.declare(metadata ptr %i, metadata !10, metadata !DIExpression()), !dbg !21
  br label %for.cond, !dbg !19

for.cond:                                         ; preds = %for.update, %for.init
  %i1 = load i64, ptr %i, align 4, !dbg !19
  %icmpsletmp = icmp sle i64 %i1, 4, !dbg !19
  br i1 %icmpsletmp, label %for.body, label %for.exit, !dbg !19

for.body:                                         ; preds = %for.cond
  %sum12 = load i64, ptr %sum1, align 4, !dbg !19
  %i3 = load i64, ptr %i, align 4, !dbg !19
  %addtmp = add i64 %sum12, %i3, !dbg !19
  store i64 %addtmp, ptr %sum1, align 4, !dbg !19
  br label %for.update, !dbg !19

for.update:                                       ; preds = %for.body
  %i4 = load i64, ptr %i, align 4, !dbg !19
  %addtmp5 = add i64 %i4, 1, !dbg !19
  store i64 %addtmp5, ptr %i, align 4, !dbg !19
  br label %for.cond, !dbg !19

for.exit:                                         ; preds = %for.cond
  %sum16 = load i64, ptr %sum1, align 4, !dbg !19
  %icmpneqtmp = icmp ne i64 %sum16, 10, !dbg !19
  br i1 %icmpneqtmp, label %then, label %ifcont, !dbg !19

then:                                             ; preds = %for.exit
  ret i64 1, !dbg !19

ifcont:                                           ; preds = %for.exit
  store i64 0, ptr %sum2, align 4, !dbg !19
  call void @llvm.dbg.declare(metadata ptr %sum2, metadata !11, metadata !DIExpression()), !dbg !22
  br label %for.init7, !dbg !19

for.init7:                                        ; preds = %ifcont
  store i64 0, ptr %i12, align 4, !dbg !19
  call void @llvm.dbg.declare(metadata ptr %i12, metadata !12, metadata !DIExpression()), !dbg !23
  br label %for.cond8, !dbg !19

for.cond8:                                        ; preds = %for.update10, %for.init7
  %i13 = load i64, ptr %i12, align 4, !dbg !19
  %icmpsletmp14 = icmp sle i64 %i13, 10, !dbg !19
  br i1 %icmpsletmp14, label %for.body9, label %for.exit11, !dbg !19

for.body9:                                        ; preds = %for.cond8
  %sum215 = load i64, ptr %sum2, align 4, !dbg !19
  %i16 = load i64, ptr %i12, align 4, !dbg !19
  %addtmp17 = add i64 %sum215, %i16, !dbg !19
  store i64 %addtmp17, ptr %sum2, align 4, !dbg !19
  br label %for.update10, !dbg !19

for.update10:                                     ; preds = %for.body9
  %i18 = load i64, ptr %i12, align 4, !dbg !19
  %addtmp19 = add i64 %i18, 2, !dbg !19
  store i64 %addtmp19, ptr %i12, align 4, !dbg !19
  br label %for.cond8, !dbg !19

for.exit11:                                       ; preds = %for.cond8
  %sum220 = load i64, ptr %sum2, align 4, !dbg !19
  %icmpneqtmp21 = icmp ne i64 %sum220, 30, !dbg !19
  br i1 %icmpneqtmp21, label %then22, label %ifcont23, !dbg !19

then22:                                           ; preds = %for.exit11
  ret i64 2, !dbg !19

ifcont23:                                         ; preds = %for.exit11
  store i64 0, ptr %sum3, align 4, !dbg !19
  call void @llvm.dbg.declare(metadata ptr %sum3, metadata !13, metadata !DIExpression()), !dbg !24
  br label %for.init24, !dbg !19

for.init24:                                       ; preds = %ifcont23
  store i64 0, ptr %i29, align 4, !dbg !19
  call void @llvm.dbg.declare(metadata ptr %i29, metadata !14, metadata !DIExpression()), !dbg !25
  br label %for.cond25, !dbg !19

for.cond25:                                       ; preds = %for.update27, %for.init24
  %i30 = load i64, ptr %i29, align 4, !dbg !19
  %icmpsletmp31 = icmp sle i64 %i30, 10, !dbg !19
  br i1 %icmpsletmp31, label %for.body26, label %for.exit28, !dbg !19

for.body26:                                       ; preds = %for.cond25
  %i32 = load i64, ptr %i29, align 4, !dbg !19
  %icmpsgttmp = icmp sgt i64 %i32, 3, !dbg !19
  br i1 %icmpsgttmp, label %then33, label %ifcont34, !dbg !19

for.update27:                                     ; preds = %ifcont34
  %i38 = load i64, ptr %i29, align 4, !dbg !19
  %addtmp39 = add i64 %i38, 1, !dbg !19
  store i64 %addtmp39, ptr %i29, align 4, !dbg !19
  br label %for.cond25, !dbg !19

for.exit28:                                       ; preds = %then33, %for.cond25
  %sum340 = load i64, ptr %sum3, align 4, !dbg !19
  %icmpneqtmp41 = icmp ne i64 %sum340, 6, !dbg !19
  br i1 %icmpneqtmp41, label %then42, label %ifcont43, !dbg !19

then33:                                           ; preds = %for.body26
  br label %for.exit28, !dbg !19

ifcont34:                                         ; preds = %for.body26
  %sum335 = load i64, ptr %sum3, align 4, !dbg !19
  %i36 = load i64, ptr %i29, align 4, !dbg !19
  %addtmp37 = add i64 %sum335, %i36, !dbg !19
  store i64 %addtmp37, ptr %sum3, align 4, !dbg !19
  br label %for.update27, !dbg !19

then42:                                           ; preds = %for.exit28
  ret i64 3, !dbg !19

ifcont43:                                         ; preds = %for.exit28
  store i64 0, ptr %sum4, align 4, !dbg !19
  call void @llvm.dbg.declare(metadata ptr %sum4, metadata !15, metadata !DIExpression()), !dbg !26
  br label %for.init44, !dbg !19

for.init44:                                       ; preds = %ifcont43
  store i64 0, ptr %i49, align 4, !dbg !19
  call void @llvm.dbg.declare(metadata ptr %i49, metadata !16, metadata !DIExpression()), !dbg !27
  br label %for.cond45, !dbg !19

for.cond45:                                       ; preds = %for.update47, %for.init44
  %i50 = load i64, ptr %i49, align 4, !dbg !19
  %icmpsletmp51 = icmp sle i64 %i50, 10, !dbg !19
  br i1 %icmpsletmp51, label %for.body46, label %for.exit48, !dbg !19

for.body46:                                       ; preds = %for.cond45
  %i52 = load i64, ptr %i49, align 4, !dbg !19
  %icmpeqtmp = icmp eq i64 %i52, 5, !dbg !19
  br i1 %icmpeqtmp, label %then53, label %ifcont54, !dbg !19

for.update47:                                     ; preds = %ifcont54, %then53
  %i58 = load i64, ptr %i49, align 4, !dbg !19
  %addtmp59 = add i64 %i58, 1, !dbg !19
  store i64 %addtmp59, ptr %i49, align 4, !dbg !19
  br label %for.cond45, !dbg !19

for.exit48:                                       ; preds = %for.cond45
  %sum460 = load i64, ptr %sum4, align 4, !dbg !19
  %icmpneqtmp61 = icmp ne i64 %sum460, 50, !dbg !19
  br i1 %icmpneqtmp61, label %then62, label %ifcont63, !dbg !19

then53:                                           ; preds = %for.body46
  br label %for.update47, !dbg !19

ifcont54:                                         ; preds = %for.body46
  %sum455 = load i64, ptr %sum4, align 4, !dbg !19
  %i56 = load i64, ptr %i49, align 4, !dbg !19
  %addtmp57 = add i64 %sum455, %i56, !dbg !19
  store i64 %addtmp57, ptr %sum4, align 4, !dbg !19
  br label %for.update47, !dbg !19

then62:                                           ; preds = %for.exit48
  ret i64 4, !dbg !19

ifcont63:                                         ; preds = %for.exit48
  store i64 0, ptr %sum5, align 4, !dbg !19
  call void @llvm.dbg.declare(metadata ptr %sum5, metadata !17, metadata !DIExpression()), !dbg !28
  br label %for.init64, !dbg !19

for.init64:                                       ; preds = %ifcont63
  store i64 -2, ptr %i69, align 4, !dbg !19
  call void @llvm.dbg.declare(metadata ptr %i69, metadata !18, metadata !DIExpression()), !dbg !29
  br label %for.cond65, !dbg !19

for.cond65:                                       ; preds = %for.update67, %for.init64
  %i70 = load i64, ptr %i69, align 4, !dbg !19
  %icmpsletmp71 = icmp sle i64 %i70, 2, !dbg !19
  br i1 %icmpsletmp71, label %for.body66, label %for.exit68, !dbg !19

for.body66:                                       ; preds = %for.cond65
  %sum572 = load i64, ptr %sum5, align 4, !dbg !19
  %i73 = load i64, ptr %i69, align 4, !dbg !19
  %addtmp74 = add i64 %sum572, %i73, !dbg !19
  store i64 %addtmp74, ptr %sum5, align 4, !dbg !19
  br label %for.update67, !dbg !19

for.update67:                                     ; preds = %for.body66
  %i75 = load i64, ptr %i69, align 4, !dbg !19
  %addtmp76 = add i64 %i75, 1, !dbg !19
  store i64 %addtmp76, ptr %i69, align 4, !dbg !19
  br label %for.cond65, !dbg !19

for.exit68:                                       ; preds = %for.cond65
  %sum577 = load i64, ptr %sum5, align 4, !dbg !19
  %icmpneqtmp78 = icmp ne i64 %sum577, 0, !dbg !19
  br i1 %icmpneqtmp78, label %then79, label %ifcont80, !dbg !19

then79:                                           ; preds = %for.exit68
  ret i64 5, !dbg !19

ifcont80:                                         ; preds = %for.exit68
  ret i64 0, !dbg !19
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
!1 = !DIFile(filename: "comprehensive_range_test.vyn.ll", directory: "/home/rick/Projects/Vyn/test/vec_for")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !10, !11, !12, !13, !14, !15, !16, !17, !18}
!9 = !DILocalVariable(name: "sum1", scope: !4, file: !1, line: 4, type: !7)
!10 = !DILocalVariable(name: "i", scope: !4, file: !1, line: 6, type: !7)
!11 = !DILocalVariable(name: "sum2", scope: !4, file: !1, line: 13, type: !7)
!12 = !DILocalVariable(name: "i", scope: !4, file: !1, line: 15, type: !7)
!13 = !DILocalVariable(name: "sum3", scope: !4, file: !1, line: 22, type: !7)
!14 = !DILocalVariable(name: "i", scope: !4, file: !1, line: 24, type: !7)
!15 = !DILocalVariable(name: "sum4", scope: !4, file: !1, line: 34, type: !7)
!16 = !DILocalVariable(name: "i", scope: !4, file: !1, line: 36, type: !7)
!17 = !DILocalVariable(name: "sum5", scope: !4, file: !1, line: 46, type: !7)
!18 = !DILocalVariable(name: "i", scope: !4, file: !1, line: 48, type: !7)
!19 = !DILocation(line: 3, column: 1, scope: !4)
!20 = !DILocation(line: 4, column: 5, scope: !4)
!21 = !DILocation(line: 6, column: 10, scope: !4)
!22 = !DILocation(line: 13, column: 5, scope: !4)
!23 = !DILocation(line: 15, column: 10, scope: !4)
!24 = !DILocation(line: 22, column: 5, scope: !4)
!25 = !DILocation(line: 24, column: 10, scope: !4)
!26 = !DILocation(line: 34, column: 5, scope: !4)
!27 = !DILocation(line: 36, column: 10, scope: !4)
!28 = !DILocation(line: 46, column: 5, scope: !4)
!29 = !DILocation(line: 48, column: 10, scope: !4)
