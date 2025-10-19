; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [16 x i8] c"Hello from test\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@1 = private unnamed_addr constant [16 x i8] c"Hello from main\00", align 1
@type_name.1 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@2 = private unnamed_addr constant [5 x i8] c"Done\00", align 1
@type_name.2 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1

define i64 @test() !dbg !4 {
entry:
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !8
  store { ptr, i64 } { ptr @0, i64 15 }, ptr %serialize_temp, align 8, !dbg !8
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !8
  call void @__vyn_println(ptr %serialized_json), !dbg !8
  ret i64 42, !dbg !8
}

define i64 @main() !dbg !9 {
entry:
  %result = alloca i64, align 8, !dbg !12
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !12
  store { ptr, i64 } { ptr @1, i64 15 }, ptr %serialize_temp, align 8, !dbg !12
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name.1), !dbg !12
  call void @__vyn_println(ptr %serialized_json), !dbg !12
  %calltmp = call i64 @test(), !dbg !12
  store i64 %calltmp, ptr %result, align 4, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %result, metadata !11, metadata !DIExpression()), !dbg !13
  %serialize_temp1 = alloca { ptr, i64 }, align 8, !dbg !12
  store { ptr, i64 } { ptr @2, i64 4 }, ptr %serialize_temp1, align 8, !dbg !12
  %serialized_json2 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp1, ptr @type_name.2), !dbg !12
  call void @__vyn_println(ptr %serialized_json2), !dbg !12
  %result3 = load i64, ptr %result, align 4, !dbg !12
  ret i64 %result3, !dbg !12
}

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "test_int_return.vyn.ll", directory: "/home/rick/Projects/Vyn/test")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "test", linkageName: "test", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !DILocation(line: 3, column: 1, scope: !4)
!9 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 8, type: !5, scopeLine: 8, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !10)
!10 = !{!11}
!11 = !DILocalVariable(name: "result", scope: !9, file: !1, line: 10, type: !7)
!12 = !DILocation(line: 8, column: 1, scope: !9)
!13 = !DILocation(line: 10, column: 1, scope: !9)
