; ModuleID = 'VynModule'
source_filename = "VynModule"

%TestStruct = type { i64 }

@0 = private unnamed_addr constant [9 x i8] c"Before: \00", align 1
@1 = private unnamed_addr constant [8 x i8] c"After: \00", align 1

define void @modify_struct(ptr %s) !dbg !4 {
entry:
  %s1 = alloca ptr, align 8
  store ptr %s, ptr %s1, align 8, !dbg !10
  call void @llvm.dbg.declare(metadata ptr %s1, metadata !9, metadata !DIExpression()), !dbg !11
  %s2 = load ptr, ptr %s1, align 8, !dbg !10
  %value_ptr = getelementptr inbounds %TestStruct, ptr %s2, i32 0, i32 0, !dbg !10
  store i64 42, ptr %value_ptr, align 4, !dbg !10
  ret void, !dbg !10
}

define i64 @main() !dbg !12 {
entry:
  %my_struct = alloca %TestStruct, align 8, !dbg !19
  %TestStruct_obj = alloca %TestStruct, align 8, !dbg !19
  %value_ptr = getelementptr inbounds %TestStruct, ptr %TestStruct_obj, i32 0, i32 0, !dbg !19
  store i64 0, ptr %value_ptr, align 4, !dbg !19
  %TestStruct_val = load %TestStruct, ptr %TestStruct_obj, align 4, !dbg !19
  store %TestStruct %TestStruct_val, ptr %my_struct, align 4, !dbg !19
  call void @llvm.dbg.declare(metadata ptr %my_struct, metadata !17, metadata !DIExpression()), !dbg !20
  %my_struct1 = load %TestStruct, ptr %my_struct, align 4, !dbg !19
  %temp_struct = alloca %TestStruct, align 8, !dbg !19
  store %TestStruct %my_struct1, ptr %temp_struct, align 4, !dbg !19
  %value_ptr2 = getelementptr inbounds %TestStruct, ptr %temp_struct, i32 0, i32 0, !dbg !19
  %value_val = load i64, ptr %value_ptr2, align 4, !dbg !19
  %tostring = call ptr @__vyn_toString_int(i64 %value_val), !dbg !19
  %strcattmp = call ptr @__vyn_string_concat(ptr @0, ptr %tostring), !dbg !19
  call void @__vyn_println(ptr %strcattmp), !dbg !19
  call void @modify_struct(ptr %my_struct), !dbg !19
  %my_struct3 = load %TestStruct, ptr %my_struct, align 4, !dbg !19
  %temp_struct4 = alloca %TestStruct, align 8, !dbg !19
  store %TestStruct %my_struct3, ptr %temp_struct4, align 4, !dbg !19
  %value_ptr5 = getelementptr inbounds %TestStruct, ptr %temp_struct4, i32 0, i32 0, !dbg !19
  %value_val6 = load i64, ptr %value_ptr5, align 4, !dbg !19
  %tostring7 = call ptr @__vyn_toString_int(i64 %value_val6), !dbg !19
  %strcattmp8 = call ptr @__vyn_string_concat(ptr @1, ptr %tostring7), !dbg !19
  call void @__vyn_println(ptr %strcattmp8), !dbg !19
  ret i64 0, !dbg !19
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @__vyn_toString_int(i64)

declare ptr @__vyn_string_concat(ptr, ptr)

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "test_borrow.vyn.ll", directory: "/home/rick/Projects/Vyn")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "modify_struct", linkageName: "modify_struct", scope: !1, file: !1, line: 6, type: !5, scopeLine: 6, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{null, !7}
!7 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!8 = !{!9}
!9 = !DILocalVariable(name: "s", scope: !4, file: !1, line: 6, type: !7)
!10 = !DILocation(line: 6, column: 1, scope: !4)
!11 = !DILocation(line: 6, column: 16, scope: !4)
!12 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 10, type: !13, scopeLine: 10, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !16)
!13 = !DISubroutineType(types: !14)
!14 = !{!15}
!15 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!16 = !{!17}
!17 = !DILocalVariable(name: "my_struct", scope: !12, file: !1, line: 11, type: !18)
!18 = !DICompositeType(tag: DW_TAG_structure_type, name: "TestStruct", scope: !1, file: !1, size: 64, align: 8)
!19 = !DILocation(line: 10, column: 1, scope: !12)
!20 = !DILocation(line: 11, column: 1, scope: !12)
