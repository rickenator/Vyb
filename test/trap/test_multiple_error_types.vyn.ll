; ModuleID = 'VynModule'
source_filename = "VynModule"

%ErrorA = type { i64 }
%ErrorB = type { i64 }

define { i64, ptr } @test_error(i64 %n) !dbg !4 {
entry:
  %n1 = alloca i64, align 8
  store i64 %n, ptr %n1, align 4, !dbg !11
  call void @llvm.dbg.declare(metadata ptr %n1, metadata !10, metadata !DIExpression()), !dbg !12
  %n2 = load i64, ptr %n1, align 4, !dbg !11
  %icmpeqtmp = icmp eq i64 %n2, 1, !dbg !11
  br i1 %icmpeqtmp, label %then, label %ifcont, !dbg !11

then:                                             ; preds = %entry
  %ErrorA_obj = alloca %ErrorA, align 8, !dbg !11
  %code_ptr = getelementptr inbounds %ErrorA, ptr %ErrorA_obj, i32 0, i32 0, !dbg !11
  store i64 42, ptr %code_ptr, align 4, !dbg !11
  %ErrorA_val = load %ErrorA, ptr %ErrorA_obj, align 4, !dbg !11
  %error.heap = call ptr @malloc(i64 16), !dbg !11
  %error.typeid.ptr = getelementptr i64, ptr %error.heap, i32 0, !dbg !11
  store i64 -1117103444634275797, ptr %error.typeid.ptr, align 4, !dbg !11
  %error.data.ptr = getelementptr i8, ptr %error.heap, i64 8, !dbg !11
  store %ErrorA %ErrorA_val, ptr %error.data.ptr, align 4, !dbg !11
  %error.ptr = insertvalue { i64, ptr } undef, ptr %error.heap, 1, !dbg !11
  ret { i64, ptr } %error.ptr, !dbg !11

ifcont:                                           ; preds = %entry
  %n3 = load i64, ptr %n1, align 4, !dbg !11
  %icmpeqtmp4 = icmp eq i64 %n3, 2, !dbg !11
  br i1 %icmpeqtmp4, label %then5, label %ifcont10, !dbg !11

then5:                                            ; preds = %ifcont
  %ErrorB_obj = alloca %ErrorB, align 8, !dbg !11
  %value_ptr = getelementptr inbounds %ErrorB, ptr %ErrorB_obj, i32 0, i32 0, !dbg !11
  store i64 99, ptr %value_ptr, align 4, !dbg !11
  %ErrorB_val = load %ErrorB, ptr %ErrorB_obj, align 4, !dbg !11
  %error.heap6 = call ptr @malloc(i64 16), !dbg !11
  %error.typeid.ptr7 = getelementptr i64, ptr %error.heap6, i32 0, !dbg !11
  store i64 -121108893005291215, ptr %error.typeid.ptr7, align 4, !dbg !11
  %error.data.ptr8 = getelementptr i8, ptr %error.heap6, i64 8, !dbg !11
  store %ErrorB %ErrorB_val, ptr %error.data.ptr8, align 4, !dbg !11
  %error.ptr9 = insertvalue { i64, ptr } undef, ptr %error.heap6, 1, !dbg !11
  ret { i64, ptr } %error.ptr9, !dbg !11

ifcont10:                                         ; preds = %ifcont
  %n11 = load i64, ptr %n1, align 4, !dbg !11
  %multmp = mul i64 %n11, 10, !dbg !11
  %result.value = insertvalue { i64, ptr } undef, i64 %multmp, 0, !dbg !11
  %result.error = insertvalue { i64, ptr } %result.value, ptr null, 1, !dbg !11
  ret { i64, ptr } %result.error, !dbg !11
}

define i64 @main() !dbg !13 {
entry:
  %r2 = alloca i64, align 8
  %trap_error9 = alloca ptr, align 8
  %r1 = alloca i64, align 8
  %trap_error = alloca ptr, align 8
  br label %block.normal, !dbg !19

block.normal:                                     ; preds = %entry
  %calltmp = call { i64, ptr } @test_error(i64 1), !dbg !19
  %call.value = extractvalue { i64, ptr } %calltmp, 0, !dbg !19
  %call.error = extractvalue { i64, ptr } %calltmp, 1, !dbg !19
  %has.error = icmp ne ptr %call.error, null, !dbg !19
  br i1 %has.error, label %call.error1, label %call.success, !dbg !19

block.continue:                                   ; preds = %trap.unmatched, %trap.handler1, %trap.handler0, %call.success
  %block.result = phi i64 [ %call.value, %call.success ], [ %code_val, %trap.handler0 ], [ %value_val, %trap.handler1 ], [ 0, %trap.unmatched ], !dbg !19
  store i64 %block.result, ptr %r1, align 4, !dbg !19
  call void @llvm.dbg.declare(metadata ptr %r1, metadata !17, metadata !DIExpression()), !dbg !20
  br label %block.normal7, !dbg !19

trap.landing:                                     ; preds = %call.error1
  %error.ptr = load ptr, ptr %trap_error, align 8, !dbg !19
  %error.typeid = load i64, ptr %error.ptr, align 4, !dbg !19
  %type.matches = icmp eq i64 %error.typeid, -1117103444634275797, !dbg !19
  br i1 %type.matches, label %trap.handler0, label %trap.check1, !dbg !19

call.error1:                                      ; preds = %block.normal
  store ptr %call.error, ptr %trap_error, align 8, !dbg !19
  br label %trap.landing, !dbg !19

call.success:                                     ; preds = %block.normal
  br label %block.continue, !dbg !19

trap.unmatched:                                   ; preds = %trap.check1
  br label %block.continue, !dbg !19

trap.handler0:                                    ; preds = %trap.landing
  %error.data.i8ptr = getelementptr i8, ptr %error.ptr, i64 8, !dbg !19
  %error.value = load %ErrorA, ptr %error.data.i8ptr, align 4, !dbg !19
  %temp_struct = alloca %ErrorA, align 8, !dbg !19
  store %ErrorA %error.value, ptr %temp_struct, align 4, !dbg !19
  %code_ptr = getelementptr inbounds %ErrorA, ptr %temp_struct, i32 0, i32 0, !dbg !19
  %code_val = load i64, ptr %code_ptr, align 4, !dbg !19
  br label %block.continue, !dbg !19

trap.check1:                                      ; preds = %trap.landing
  %error.typeid2 = load i64, ptr %error.ptr, align 4, !dbg !19
  %type.matches3 = icmp eq i64 %error.typeid2, -121108893005291215, !dbg !19
  br i1 %type.matches3, label %trap.handler1, label %trap.unmatched, !dbg !19

trap.handler1:                                    ; preds = %trap.check1
  %error.data.i8ptr4 = getelementptr i8, ptr %error.ptr, i64 8, !dbg !19
  %error.value5 = load %ErrorB, ptr %error.data.i8ptr4, align 4, !dbg !19
  %temp_struct6 = alloca %ErrorB, align 8, !dbg !19
  store %ErrorB %error.value5, ptr %temp_struct6, align 4, !dbg !19
  %value_ptr = getelementptr inbounds %ErrorB, ptr %temp_struct6, i32 0, i32 0, !dbg !19
  %value_val = load i64, ptr %value_ptr, align 4, !dbg !19
  br label %block.continue, !dbg !19

block.normal7:                                    ; preds = %block.continue
  %calltmp11 = call { i64, ptr } @test_error(i64 2), !dbg !19
  %call.value12 = extractvalue { i64, ptr } %calltmp11, 0, !dbg !19
  %call.error13 = extractvalue { i64, ptr } %calltmp11, 1, !dbg !19
  %has.error14 = icmp ne ptr %call.error13, null, !dbg !19
  br i1 %has.error14, label %call.error15, label %call.success16, !dbg !19

block.continue8:                                  ; preds = %trap.unmatched18, %trap.handler125, %trap.handler019, %call.success16
  %block.result33 = phi i64 [ %call.value12, %call.success16 ], [ 1, %trap.handler019 ], [ %value_val32, %trap.handler125 ], [ 0, %trap.unmatched18 ], !dbg !19
  store i64 %block.result33, ptr %r2, align 4, !dbg !19
  call void @llvm.dbg.declare(metadata ptr %r2, metadata !18, metadata !DIExpression()), !dbg !21
  %r134 = load i64, ptr %r1, align 4, !dbg !19
  %r235 = load i64, ptr %r2, align 4, !dbg !19
  %addtmp = add i64 %r134, %r235, !dbg !19
  ret i64 %addtmp, !dbg !19

trap.landing10:                                   ; preds = %call.error15
  %error.ptr17 = load ptr, ptr %trap_error9, align 8, !dbg !19
  %error.typeid21 = load i64, ptr %error.ptr17, align 4, !dbg !19
  %type.matches22 = icmp eq i64 %error.typeid21, -1117103444634275797, !dbg !19
  br i1 %type.matches22, label %trap.handler019, label %trap.check120, !dbg !19

call.error15:                                     ; preds = %block.normal7
  store ptr %call.error13, ptr %trap_error9, align 8, !dbg !19
  br label %trap.landing10, !dbg !19

call.success16:                                   ; preds = %block.normal7
  br label %block.continue8, !dbg !19

trap.unmatched18:                                 ; preds = %trap.check120
  br label %block.continue8, !dbg !19

trap.handler019:                                  ; preds = %trap.landing10
  %error.data.i8ptr23 = getelementptr i8, ptr %error.ptr17, i64 8, !dbg !19
  %error.value24 = load %ErrorA, ptr %error.data.i8ptr23, align 4, !dbg !19
  br label %block.continue8, !dbg !19

trap.check120:                                    ; preds = %trap.landing10
  %error.typeid26 = load i64, ptr %error.ptr17, align 4, !dbg !19
  %type.matches27 = icmp eq i64 %error.typeid26, -121108893005291215, !dbg !19
  br i1 %type.matches27, label %trap.handler125, label %trap.unmatched18, !dbg !19

trap.handler125:                                  ; preds = %trap.check120
  %error.data.i8ptr28 = getelementptr i8, ptr %error.ptr17, i64 8, !dbg !19
  %error.value29 = load %ErrorB, ptr %error.data.i8ptr28, align 4, !dbg !19
  %temp_struct30 = alloca %ErrorB, align 8, !dbg !19
  store %ErrorB %error.value29, ptr %temp_struct30, align 4, !dbg !19
  %value_ptr31 = getelementptr inbounds %ErrorB, ptr %temp_struct30, i32 0, i32 0, !dbg !19
  %value_val32 = load i64, ptr %value_ptr31, align 4, !dbg !19
  br label %block.continue8, !dbg !19
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @malloc(i64)

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "test_multiple_error_types.vyn.ll", directory: "/home/rick/Projects/Vyn/test/trap")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "test_error", linkageName: "test_error", scope: !1, file: !1, line: 12, type: !5, scopeLine: 12, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !9)
!5 = !DISubroutineType(types: !6)
!6 = !{!7, !8}
!7 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_return", scope: !1, file: !1, size: 128, align: 8)
!8 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!9 = !{!10}
!10 = !DILocalVariable(name: "n", scope: !4, file: !1, line: 12, type: !8)
!11 = !DILocation(line: 12, column: 1, scope: !4)
!12 = !DILocation(line: 12, column: 13, scope: !4)
!13 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 22, type: !14, scopeLine: 22, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !16)
!14 = !DISubroutineType(types: !15)
!15 = !{!8}
!16 = !{!17, !18}
!17 = !DILocalVariable(name: "r1", scope: !13, file: !1, line: 23, type: !8)
!18 = !DILocalVariable(name: "r2", scope: !13, file: !1, line: 31, type: !8)
!19 = !DILocation(line: 22, column: 1, scope: !13)
!20 = !DILocation(line: 23, column: 1, scope: !13)
!21 = !DILocation(line: 31, column: 1, scope: !13)
