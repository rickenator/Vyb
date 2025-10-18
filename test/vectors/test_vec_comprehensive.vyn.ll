; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [21 x i8] c"After pushing 1,2,3:\00", align 1
@1 = private unnamed_addr constant [14 x i8] c"After concat:\00", align 1
@2 = private unnamed_addr constant [13 x i8] c"After clear:\00", align 1

define i64 @main() !dbg !4 {
entry:
  %i = alloca i64, align 8, !dbg !20
  %big_vec = alloca { ptr, i64, i64 }, align 8, !dbg !20
  %vec2 = alloca { ptr, i64, i64 }, align 8, !dbg !20
  %has_five = alloca i1, align 1, !dbg !20
  %has_two = alloca i1, align 1, !dbg !20
  %third = alloca i64, align 8, !dbg !20
  %second = alloca i64, align 8, !dbg !20
  %first = alloca i64, align 8, !dbg !20
  %vec = alloca { ptr, i64, i64 }, align 8, !dbg !20
  %vec.new = alloca { ptr, i64, i64 }, align 8, !dbg !20
  %vec.ptr_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 0, !dbg !20
  store ptr null, ptr %vec.ptr_field, align 8, !dbg !20
  %vec.size_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 1, !dbg !20
  store i64 0, ptr %vec.size_field, align 4, !dbg !20
  %vec.cap_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 2, !dbg !20
  store i64 0, ptr %vec.cap_field, align 4, !dbg !20
  %vec.new.value = load { ptr, i64, i64 }, ptr %vec.new, align 8, !dbg !20
  store { ptr, i64, i64 } %vec.new.value, ptr %vec, align 8, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %vec, metadata !9, metadata !DIExpression()), !dbg !21
  call void @__vyn_println(ptr @0), !dbg !20
  store i64 42, ptr %first, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %first, metadata !11, metadata !DIExpression()), !dbg !22
  store i64 42, ptr %second, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %second, metadata !12, metadata !DIExpression()), !dbg !23
  store i64 42, ptr %third, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %third, metadata !13, metadata !DIExpression()), !dbg !24
  store i1 false, ptr %has_two, align 1, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %has_two, metadata !14, metadata !DIExpression()), !dbg !25
  store i1 false, ptr %has_five, align 1, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %has_five, metadata !16, metadata !DIExpression()), !dbg !26
  %vec.new1 = alloca { ptr, i64, i64 }, align 8, !dbg !20
  %vec.ptr_field2 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new1, i32 0, i32 0, !dbg !20
  store ptr null, ptr %vec.ptr_field2, align 8, !dbg !20
  %vec.size_field3 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new1, i32 0, i32 1, !dbg !20
  store i64 0, ptr %vec.size_field3, align 4, !dbg !20
  %vec.cap_field4 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new1, i32 0, i32 2, !dbg !20
  store i64 0, ptr %vec.cap_field4, align 4, !dbg !20
  %vec.new.value5 = load { ptr, i64, i64 }, ptr %vec.new1, align 8, !dbg !20
  store { ptr, i64, i64 } %vec.new.value5, ptr %vec2, align 8, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %vec2, metadata !17, metadata !DIExpression()), !dbg !27
  %vec26 = load { ptr, i64, i64 }, ptr %vec2, align 8, !dbg !20
  call void @__vyn_println(ptr @1), !dbg !20
  call void @__vyn_println(ptr @2), !dbg !20
  %vec.new7 = alloca { ptr, i64, i64 }, align 8, !dbg !20
  %vec.ptr_field8 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new7, i32 0, i32 0, !dbg !20
  store ptr null, ptr %vec.ptr_field8, align 8, !dbg !20
  %vec.size_field9 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new7, i32 0, i32 1, !dbg !20
  store i64 0, ptr %vec.size_field9, align 4, !dbg !20
  %vec.cap_field10 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new7, i32 0, i32 2, !dbg !20
  store i64 0, ptr %vec.cap_field10, align 4, !dbg !20
  %vec.new.value11 = load { ptr, i64, i64 }, ptr %vec.new7, align 8, !dbg !20
  store { ptr, i64, i64 } %vec.new.value11, ptr %big_vec, align 8, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %big_vec, metadata !18, metadata !DIExpression()), !dbg !28
  store i64 0, ptr %i, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %i, metadata !19, metadata !DIExpression()), !dbg !29
  br label %loop.header, !dbg !20

loop.header:                                      ; preds = %loop.body, %entry
  %i12 = load i64, ptr %i, align 4, !dbg !20
  %icmpslttmp = icmp slt i64 %i12, 10, !dbg !20
  br i1 %icmpslttmp, label %loop.body, label %loop.exit, !dbg !20

loop.body:                                        ; preds = %loop.header
  %i13 = load i64, ptr %i, align 4, !dbg !20
  %i14 = load i64, ptr %i, align 4, !dbg !20
  %multmp = mul i64 %i13, %i14, !dbg !20
  %vec.size_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %big_vec, i32 0, i32 1, !dbg !20
  %vec.current_size = load i64, ptr %vec.size_ptr, align 4, !dbg !20
  %vec.new_size = add i64 %vec.current_size, 1, !dbg !20
  store i64 %vec.new_size, ptr %vec.size_ptr, align 4, !dbg !20
  %i15 = load i64, ptr %i, align 4, !dbg !20
  %addtmp = add i64 %i15, 1, !dbg !20
  store i64 %addtmp, ptr %i, align 4, !dbg !20
  br label %loop.header, !dbg !20

loop.exit:                                        ; preds = %loop.header
  %big_vec_cleanup_load = load { ptr, i64, i64 }, ptr %big_vec, align 8, !dbg !20
  %big_vec_data_ptr = extractvalue { ptr, i64, i64 } %big_vec_cleanup_load, 0, !dbg !20
  %big_vec_null_check = icmp ne ptr %big_vec_data_ptr, null, !dbg !20
  br i1 %big_vec_null_check, label %big_vec_free_block, label %big_vec_continue, !dbg !20

big_vec_free_block:                               ; preds = %loop.exit
  call void @free(ptr %big_vec_data_ptr), !dbg !20
  br label %big_vec_continue, !dbg !20

big_vec_continue:                                 ; preds = %big_vec_free_block, %loop.exit
  %vec2_cleanup_load = load { ptr, i64, i64 }, ptr %vec2, align 8, !dbg !20
  %vec2_data_ptr = extractvalue { ptr, i64, i64 } %vec2_cleanup_load, 0, !dbg !20
  %vec2_null_check = icmp ne ptr %vec2_data_ptr, null, !dbg !20
  br i1 %vec2_null_check, label %vec2_free_block, label %vec2_continue, !dbg !20

vec2_free_block:                                  ; preds = %big_vec_continue
  call void @free(ptr %vec2_data_ptr), !dbg !20
  br label %vec2_continue, !dbg !20

vec2_continue:                                    ; preds = %vec2_free_block, %big_vec_continue
  %vec_cleanup_load = load { ptr, i64, i64 }, ptr %vec, align 8, !dbg !20
  %vec_data_ptr = extractvalue { ptr, i64, i64 } %vec_cleanup_load, 0, !dbg !20
  %vec_null_check = icmp ne ptr %vec_data_ptr, null, !dbg !20
  br i1 %vec_null_check, label %vec_free_block, label %vec_continue, !dbg !20

vec_free_block:                                   ; preds = %vec2_continue
  call void @free(ptr %vec_data_ptr), !dbg !20
  br label %vec_continue, !dbg !20

vec_continue:                                     ; preds = %vec_free_block, %vec2_continue
  ret i64 0, !dbg !20
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare void @println(ptr)

declare void @__vyn_println(ptr)

declare void @free(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "test_vec_comprehensive.vyn.ll", directory: "/home/rick/Projects/Vyn/test/vectors")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 2, type: !5, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11, !12, !13, !14, !16, !17, !18, !19}
!9 = !DILocalVariable(name: "vec", scope: !4, file: !1, line: 3, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_{ ptr, i64, i64 }", scope: !1, file: !1, size: 192, align: 8)
!11 = !DILocalVariable(name: "first", scope: !4, file: !1, line: 16, type: !7)
!12 = !DILocalVariable(name: "second", scope: !4, file: !1, line: 18, type: !7)
!13 = !DILocalVariable(name: "third", scope: !4, file: !1, line: 19, type: !7)
!14 = !DILocalVariable(name: "has_two", scope: !4, file: !1, line: 22, type: !15)
!15 = !DIBasicType(name: "bool", size: 1, encoding: DW_ATE_boolean)
!16 = !DILocalVariable(name: "has_five", scope: !4, file: !1, line: 24, type: !15)
!17 = !DILocalVariable(name: "vec2", scope: !4, file: !1, line: 28, type: !10)
!18 = !DILocalVariable(name: "big_vec", scope: !4, file: !1, line: 49, type: !10)
!19 = !DILocalVariable(name: "i", scope: !4, file: !1, line: 51, type: !7)
!20 = !DILocation(line: 2, column: 1, scope: !4)
!21 = !DILocation(line: 3, column: 5, scope: !4)
!22 = !DILocation(line: 16, column: 5, scope: !4)
!23 = !DILocation(line: 18, column: 1, scope: !4)
!24 = !DILocation(line: 19, column: 1, scope: !4)
!25 = !DILocation(line: 22, column: 5, scope: !4)
!26 = !DILocation(line: 24, column: 1, scope: !4)
!27 = !DILocation(line: 28, column: 5, scope: !4)
!28 = !DILocation(line: 49, column: 5, scope: !4)
!29 = !DILocation(line: 51, column: 1, scope: !4)
