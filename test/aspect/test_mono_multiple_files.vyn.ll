; ModuleID = 'VynModule'
source_filename = "VynModule"

%Box_Int = type { i64 }
%Result_Bool_Int = type { i1, i64 }
%Result_Int_String = type { i64, { ptr, i64 } }
%Box_String = type { { ptr, i64 } }
%Box_Bool = type { i1 }

@0 = private unnamed_addr constant [5 x i8] c"test\00", align 1
@1 = private unnamed_addr constant [5 x i8] c"none\00", align 1

define i64 @main() !dbg !4 {
entry:
  %b4 = alloca %Box_Int, align 8, !dbg !20
  %r2 = alloca %Result_Bool_Int, align 8, !dbg !20
  %r1 = alloca %Result_Int_String, align 8, !dbg !20
  %b3 = alloca %Box_String, align 8, !dbg !20
  %b2 = alloca %Box_Bool, align 8, !dbg !20
  %b1 = alloca %Box_Int, align 8, !dbg !20
  %Box_Int_obj = alloca %Box_Int, align 8, !dbg !20
  %value_ptr = getelementptr inbounds %Box_Int, ptr %Box_Int_obj, i32 0, i32 0, !dbg !20
  store i64 1, ptr %value_ptr, align 4, !dbg !20
  %Box_Int_val = load %Box_Int, ptr %Box_Int_obj, align 4, !dbg !20
  store %Box_Int %Box_Int_val, ptr %b1, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %b1, metadata !9, metadata !DIExpression()), !dbg !21
  %Box_Bool_obj = alloca %Box_Bool, align 8, !dbg !20
  %value_ptr1 = getelementptr inbounds %Box_Bool, ptr %Box_Bool_obj, i32 0, i32 0, !dbg !20
  store i1 true, ptr %value_ptr1, align 1, !dbg !20
  %Box_Bool_val = load %Box_Bool, ptr %Box_Bool_obj, align 1, !dbg !20
  store %Box_Bool %Box_Bool_val, ptr %b2, align 1, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %b2, metadata !11, metadata !DIExpression()), !dbg !22
  %Box_String_obj = alloca %Box_String, align 8, !dbg !20
  %value_ptr2 = getelementptr inbounds %Box_String, ptr %Box_String_obj, i32 0, i32 0, !dbg !20
  store { ptr, i64 } { ptr @0, i64 4 }, ptr %value_ptr2, align 8, !dbg !20
  %Box_String_val = load %Box_String, ptr %Box_String_obj, align 8, !dbg !20
  store %Box_String %Box_String_val, ptr %b3, align 8, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %b3, metadata !13, metadata !DIExpression()), !dbg !23
  %Result_Int_String_obj = alloca %Result_Int_String, align 8, !dbg !20
  %ok_ptr = getelementptr inbounds %Result_Int_String, ptr %Result_Int_String_obj, i32 0, i32 0, !dbg !20
  store i64 42, ptr %ok_ptr, align 4, !dbg !20
  %err_ptr = getelementptr inbounds %Result_Int_String, ptr %Result_Int_String_obj, i32 0, i32 1, !dbg !20
  store { ptr, i64 } { ptr @1, i64 4 }, ptr %err_ptr, align 8, !dbg !20
  %Result_Int_String_val = load %Result_Int_String, ptr %Result_Int_String_obj, align 8, !dbg !20
  store %Result_Int_String %Result_Int_String_val, ptr %r1, align 8, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %r1, metadata !15, metadata !DIExpression()), !dbg !24
  %Result_Bool_Int_obj = alloca %Result_Bool_Int, align 8, !dbg !20
  %ok_ptr3 = getelementptr inbounds %Result_Bool_Int, ptr %Result_Bool_Int_obj, i32 0, i32 0, !dbg !20
  store i1 true, ptr %ok_ptr3, align 1, !dbg !20
  %err_ptr4 = getelementptr inbounds %Result_Bool_Int, ptr %Result_Bool_Int_obj, i32 0, i32 1, !dbg !20
  store i64 0, ptr %err_ptr4, align 4, !dbg !20
  %Result_Bool_Int_val = load %Result_Bool_Int, ptr %Result_Bool_Int_obj, align 4, !dbg !20
  store %Result_Bool_Int %Result_Bool_Int_val, ptr %r2, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %r2, metadata !17, metadata !DIExpression()), !dbg !25
  %Box_Int_obj5 = alloca %Box_Int, align 8, !dbg !20
  %value_ptr6 = getelementptr inbounds %Box_Int, ptr %Box_Int_obj5, i32 0, i32 0, !dbg !20
  store i64 2, ptr %value_ptr6, align 4, !dbg !20
  %Box_Int_val7 = load %Box_Int, ptr %Box_Int_obj5, align 4, !dbg !20
  store %Box_Int %Box_Int_val7, ptr %b4, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %b4, metadata !19, metadata !DIExpression()), !dbg !26
  ret i64 0, !dbg !20
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
!1 = !DIFile(filename: "test_mono_multiple_files.vyn.ll", directory: "/home/rick/Projects/Vyn/test/aspect")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 13, type: !5, scopeLine: 13, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11, !13, !15, !17, !19}
!9 = !DILocalVariable(name: "b1", scope: !4, file: !1, line: 15, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "Box_Int", scope: !1, file: !1, size: 64, align: 8)
!11 = !DILocalVariable(name: "b2", scope: !4, file: !1, line: 16, type: !12)
!12 = !DICompositeType(tag: DW_TAG_structure_type, name: "Box_Bool", scope: !1, file: !1, size: 64, align: 8)
!13 = !DILocalVariable(name: "b3", scope: !4, file: !1, line: 17, type: !14)
!14 = !DICompositeType(tag: DW_TAG_structure_type, name: "Box_String", scope: !1, file: !1, size: 64, align: 8)
!15 = !DILocalVariable(name: "r1", scope: !4, file: !1, line: 20, type: !16)
!16 = !DICompositeType(tag: DW_TAG_structure_type, name: "Result_Int_String", scope: !1, file: !1, size: 128, align: 8)
!17 = !DILocalVariable(name: "r2", scope: !4, file: !1, line: 21, type: !18)
!18 = !DICompositeType(tag: DW_TAG_structure_type, name: "Result_Bool_Int", scope: !1, file: !1, size: 128, align: 8)
!19 = !DILocalVariable(name: "b4", scope: !4, file: !1, line: 24, type: !10)
!20 = !DILocation(line: 13, column: 1, scope: !4)
!21 = !DILocation(line: 15, column: 1, scope: !4)
!22 = !DILocation(line: 16, column: 1, scope: !4)
!23 = !DILocation(line: 17, column: 1, scope: !4)
!24 = !DILocation(line: 20, column: 1, scope: !4)
!25 = !DILocation(line: 21, column: 1, scope: !4)
!26 = !DILocation(line: 24, column: 1, scope: !4)
