; ModuleID = 'VynModule'
source_filename = "VynModule"

%Box_Int = type { i64 }
%Pair_Int_String = type { i64, { ptr, i64 } }
%Box_String = type { { ptr, i64 } }

@0 = private unnamed_addr constant [6 x i8] c"hello\00", align 1
@1 = private unnamed_addr constant [6 x i8] c"world\00", align 1

define i64 @main() !dbg !4 {
entry:
  %box_int2 = alloca %Box_Int, align 8, !dbg !16
  %pair = alloca %Pair_Int_String, align 8, !dbg !16
  %box_str = alloca %Box_String, align 8, !dbg !16
  %box_int = alloca %Box_Int, align 8, !dbg !16
  %Box_Int_obj = alloca %Box_Int, align 8, !dbg !16
  %value_ptr = getelementptr inbounds %Box_Int, ptr %Box_Int_obj, i32 0, i32 0, !dbg !16
  store i64 42, ptr %value_ptr, align 4, !dbg !16
  %Box_Int_val = load %Box_Int, ptr %Box_Int_obj, align 4, !dbg !16
  store %Box_Int %Box_Int_val, ptr %box_int, align 4, !dbg !16
  call void @llvm.dbg.declare(metadata ptr %box_int, metadata !9, metadata !DIExpression()), !dbg !17
  %Box_String_obj = alloca %Box_String, align 8, !dbg !16
  %value_ptr1 = getelementptr inbounds %Box_String, ptr %Box_String_obj, i32 0, i32 0, !dbg !16
  store { ptr, i64 } { ptr @0, i64 5 }, ptr %value_ptr1, align 8, !dbg !16
  %Box_String_val = load %Box_String, ptr %Box_String_obj, align 8, !dbg !16
  store %Box_String %Box_String_val, ptr %box_str, align 8, !dbg !16
  call void @llvm.dbg.declare(metadata ptr %box_str, metadata !11, metadata !DIExpression()), !dbg !18
  %Pair_Int_String_obj = alloca %Pair_Int_String, align 8, !dbg !16
  %first_ptr = getelementptr inbounds %Pair_Int_String, ptr %Pair_Int_String_obj, i32 0, i32 0, !dbg !16
  store i64 100, ptr %first_ptr, align 4, !dbg !16
  %second_ptr = getelementptr inbounds %Pair_Int_String, ptr %Pair_Int_String_obj, i32 0, i32 1, !dbg !16
  store { ptr, i64 } { ptr @1, i64 5 }, ptr %second_ptr, align 8, !dbg !16
  %Pair_Int_String_val = load %Pair_Int_String, ptr %Pair_Int_String_obj, align 8, !dbg !16
  store %Pair_Int_String %Pair_Int_String_val, ptr %pair, align 8, !dbg !16
  call void @llvm.dbg.declare(metadata ptr %pair, metadata !13, metadata !DIExpression()), !dbg !19
  %Box_Int_obj2 = alloca %Box_Int, align 8, !dbg !16
  %value_ptr3 = getelementptr inbounds %Box_Int, ptr %Box_Int_obj2, i32 0, i32 0, !dbg !16
  store i64 84, ptr %value_ptr3, align 4, !dbg !16
  %Box_Int_val4 = load %Box_Int, ptr %Box_Int_obj2, align 4, !dbg !16
  store %Box_Int %Box_Int_val4, ptr %box_int2, align 4, !dbg !16
  call void @llvm.dbg.declare(metadata ptr %box_int2, metadata !15, metadata !DIExpression()), !dbg !20
  ret i64 0, !dbg !16
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
!1 = !DIFile(filename: "test_mono_complete.vyn.ll", directory: "/home/rick/Projects/Vyn/test/trait")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 13, type: !5, scopeLine: 13, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11, !13, !15}
!9 = !DILocalVariable(name: "box_int", scope: !4, file: !1, line: 15, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "Box_Int", scope: !1, file: !1, size: 64, align: 8)
!11 = !DILocalVariable(name: "box_str", scope: !4, file: !1, line: 16, type: !12)
!12 = !DICompositeType(tag: DW_TAG_structure_type, name: "Box_String", scope: !1, file: !1, size: 64, align: 8)
!13 = !DILocalVariable(name: "pair", scope: !4, file: !1, line: 19, type: !14)
!14 = !DICompositeType(tag: DW_TAG_structure_type, name: "Pair_Int_String", scope: !1, file: !1, size: 128, align: 8)
!15 = !DILocalVariable(name: "box_int2", scope: !4, file: !1, line: 25, type: !10)
!16 = !DILocation(line: 13, column: 1, scope: !4)
!17 = !DILocation(line: 15, column: 1, scope: !4)
!18 = !DILocation(line: 16, column: 1, scope: !4)
!19 = !DILocation(line: 19, column: 1, scope: !4)
!20 = !DILocation(line: 25, column: 1, scope: !4)
