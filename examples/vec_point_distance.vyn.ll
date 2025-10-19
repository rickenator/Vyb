; ModuleID = 'VynModule'
source_filename = "VynModule"

%Point = type { double, double }

@0 = private unnamed_addr constant [57 x i8] c"Calculating distance squared from origin for each point:\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@1 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@type_name.1 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@type_name.2 = private unnamed_addr constant [6 x i8] c"Float\00", align 1

define i64 @main() !dbg !4 {
entry:
  %count = alloca i64, align 8, !dbg !20
  %distance_sq = alloca double, align 8, !dbg !20
  %p = alloca %Point, align 8, !dbg !20
  %__idx_p = alloca i64, align 8, !dbg !20
  %__len_p = alloca i64, align 8, !dbg !20
  %__run_once_p = alloca i1, align 1, !dbg !20
  %points = alloca { ptr, i64, i64 }, align 8, !dbg !20
  %vec.new = alloca { ptr, i64, i64 }, align 8, !dbg !20
  %vec.ptr_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 0, !dbg !20
  store ptr null, ptr %vec.ptr_field, align 8, !dbg !20
  %vec.size_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 1, !dbg !20
  store i64 0, ptr %vec.size_field, align 4, !dbg !20
  %vec.cap_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 2, !dbg !20
  store i64 0, ptr %vec.cap_field, align 4, !dbg !20
  %vec.new.value = load { ptr, i64, i64 }, ptr %vec.new, align 8, !dbg !20
  store { ptr, i64, i64 } %vec.new.value, ptr %points, align 8, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %points, metadata !9, metadata !DIExpression()), !dbg !21
  %Point_obj = alloca %Point, align 8, !dbg !20
  %x_ptr = getelementptr inbounds %Point, ptr %Point_obj, i32 0, i32 0, !dbg !20
  store double 3.000000e+00, ptr %x_ptr, align 8, !dbg !20
  %y_ptr = getelementptr inbounds %Point, ptr %Point_obj, i32 0, i32 1, !dbg !20
  store double 4.000000e+00, ptr %y_ptr, align 8, !dbg !20
  %Point_val = load %Point, ptr %Point_obj, align 8, !dbg !20
  %vec.data_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %points, i32 0, i32 0, !dbg !20
  %vec.size_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %points, i32 0, i32 1, !dbg !20
  %vec.cap_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %points, i32 0, i32 2, !dbg !20
  %vec.current_size = load i64, ptr %vec.size_ptr, align 4, !dbg !20
  %vec.current_cap = load i64, ptr %vec.cap_ptr, align 4, !dbg !20
  %vec.data = load ptr, ptr %vec.data_ptr, align 8, !dbg !20
  %vec.needs_alloc = icmp eq i64 %vec.current_cap, 0, !dbg !20
  %vec.needs_grow = icmp eq i64 %vec.current_size, %vec.current_cap, !dbg !20
  %vec.needs_realloc = or i1 %vec.needs_alloc, %vec.needs_grow, !dbg !20
  br i1 %vec.needs_realloc, label %vec.alloc, label %vec.merge, !dbg !20

vec.alloc:                                        ; preds = %entry
  %0 = mul i64 %vec.current_cap, 2, !dbg !20
  %vec.new_cap = select i1 %vec.needs_alloc, i64 4, i64 %0, !dbg !20
  %vec.alloc_size = mul i64 %vec.new_cap, 16, !dbg !20
  %vec.new_data = call ptr @malloc(i64 %vec.alloc_size), !dbg !20
  %vec.has_data = icmp ne i64 %vec.current_size, 0, !dbg !20
  br i1 %vec.has_data, label %vec.copy, label %vec.no_copy, !dbg !20

vec.copy:                                         ; preds = %vec.alloc
  %vec.copy_size = mul i64 %vec.current_size, 16, !dbg !20
  %1 = call ptr @memcpy(ptr %vec.new_data, ptr %vec.data, i64 %vec.copy_size), !dbg !20
  br label %vec.no_copy, !dbg !20

vec.no_copy:                                      ; preds = %vec.copy, %vec.alloc
  store ptr %vec.new_data, ptr %vec.data_ptr, align 8, !dbg !20
  store i64 %vec.new_cap, ptr %vec.cap_ptr, align 4, !dbg !20
  br label %vec.merge, !dbg !20

vec.merge:                                        ; preds = %vec.no_copy, %entry
  %vec.final_data = load ptr, ptr %vec.data_ptr, align 8, !dbg !20
  %vec.reloaded_size = load i64, ptr %vec.size_ptr, align 4, !dbg !20
  %vec.offset = mul i64 %vec.reloaded_size, 16, !dbg !20
  %vec.element_ptr = getelementptr i8, ptr %vec.final_data, i64 %vec.offset, !dbg !20
  %vec.temp_struct = alloca %Point, align 8, !dbg !20
  store %Point %Point_val, ptr %vec.temp_struct, align 8, !dbg !20
  %2 = call ptr @memcpy(ptr %vec.element_ptr, ptr %vec.temp_struct, i64 16), !dbg !20
  %vec.new_size = add i64 %vec.reloaded_size, 1, !dbg !20
  store i64 %vec.new_size, ptr %vec.size_ptr, align 4, !dbg !20
  %Point_obj1 = alloca %Point, align 8, !dbg !20
  %x_ptr2 = getelementptr inbounds %Point, ptr %Point_obj1, i32 0, i32 0, !dbg !20
  store double 1.000000e+00, ptr %x_ptr2, align 8, !dbg !20
  %y_ptr3 = getelementptr inbounds %Point, ptr %Point_obj1, i32 0, i32 1, !dbg !20
  store double 1.000000e+00, ptr %y_ptr3, align 8, !dbg !20
  %Point_val4 = load %Point, ptr %Point_obj1, align 8, !dbg !20
  %vec.data_ptr5 = getelementptr inbounds { ptr, i64, i64 }, ptr %points, i32 0, i32 0, !dbg !20
  %vec.size_ptr6 = getelementptr inbounds { ptr, i64, i64 }, ptr %points, i32 0, i32 1, !dbg !20
  %vec.cap_ptr7 = getelementptr inbounds { ptr, i64, i64 }, ptr %points, i32 0, i32 2, !dbg !20
  %vec.current_size8 = load i64, ptr %vec.size_ptr6, align 4, !dbg !20
  %vec.current_cap9 = load i64, ptr %vec.cap_ptr7, align 4, !dbg !20
  %vec.data10 = load ptr, ptr %vec.data_ptr5, align 8, !dbg !20
  %vec.needs_alloc11 = icmp eq i64 %vec.current_cap9, 0, !dbg !20
  %vec.needs_grow12 = icmp eq i64 %vec.current_size8, %vec.current_cap9, !dbg !20
  %vec.needs_realloc13 = or i1 %vec.needs_alloc11, %vec.needs_grow12, !dbg !20
  br i1 %vec.needs_realloc13, label %vec.alloc14, label %vec.merge17, !dbg !20

vec.alloc14:                                      ; preds = %vec.merge
  %3 = mul i64 %vec.current_cap9, 2, !dbg !20
  %vec.new_cap18 = select i1 %vec.needs_alloc11, i64 4, i64 %3, !dbg !20
  %vec.alloc_size19 = mul i64 %vec.new_cap18, 16, !dbg !20
  %vec.new_data20 = call ptr @malloc(i64 %vec.alloc_size19), !dbg !20
  %vec.has_data21 = icmp ne i64 %vec.current_size8, 0, !dbg !20
  br i1 %vec.has_data21, label %vec.copy15, label %vec.no_copy16, !dbg !20

vec.copy15:                                       ; preds = %vec.alloc14
  %vec.copy_size22 = mul i64 %vec.current_size8, 16, !dbg !20
  %4 = call ptr @memcpy(ptr %vec.new_data20, ptr %vec.data10, i64 %vec.copy_size22), !dbg !20
  br label %vec.no_copy16, !dbg !20

vec.no_copy16:                                    ; preds = %vec.copy15, %vec.alloc14
  store ptr %vec.new_data20, ptr %vec.data_ptr5, align 8, !dbg !20
  store i64 %vec.new_cap18, ptr %vec.cap_ptr7, align 4, !dbg !20
  br label %vec.merge17, !dbg !20

vec.merge17:                                      ; preds = %vec.no_copy16, %vec.merge
  %vec.final_data23 = load ptr, ptr %vec.data_ptr5, align 8, !dbg !20
  %vec.reloaded_size24 = load i64, ptr %vec.size_ptr6, align 4, !dbg !20
  %vec.offset25 = mul i64 %vec.reloaded_size24, 16, !dbg !20
  %vec.element_ptr26 = getelementptr i8, ptr %vec.final_data23, i64 %vec.offset25, !dbg !20
  %vec.temp_struct27 = alloca %Point, align 8, !dbg !20
  store %Point %Point_val4, ptr %vec.temp_struct27, align 8, !dbg !20
  %5 = call ptr @memcpy(ptr %vec.element_ptr26, ptr %vec.temp_struct27, i64 16), !dbg !20
  %vec.new_size28 = add i64 %vec.reloaded_size24, 1, !dbg !20
  store i64 %vec.new_size28, ptr %vec.size_ptr6, align 4, !dbg !20
  %Point_obj29 = alloca %Point, align 8, !dbg !20
  %x_ptr30 = getelementptr inbounds %Point, ptr %Point_obj29, i32 0, i32 0, !dbg !20
  store double 5.000000e+00, ptr %x_ptr30, align 8, !dbg !20
  %y_ptr31 = getelementptr inbounds %Point, ptr %Point_obj29, i32 0, i32 1, !dbg !20
  store double 1.200000e+01, ptr %y_ptr31, align 8, !dbg !20
  %Point_val32 = load %Point, ptr %Point_obj29, align 8, !dbg !20
  %vec.data_ptr33 = getelementptr inbounds { ptr, i64, i64 }, ptr %points, i32 0, i32 0, !dbg !20
  %vec.size_ptr34 = getelementptr inbounds { ptr, i64, i64 }, ptr %points, i32 0, i32 1, !dbg !20
  %vec.cap_ptr35 = getelementptr inbounds { ptr, i64, i64 }, ptr %points, i32 0, i32 2, !dbg !20
  %vec.current_size36 = load i64, ptr %vec.size_ptr34, align 4, !dbg !20
  %vec.current_cap37 = load i64, ptr %vec.cap_ptr35, align 4, !dbg !20
  %vec.data38 = load ptr, ptr %vec.data_ptr33, align 8, !dbg !20
  %vec.needs_alloc39 = icmp eq i64 %vec.current_cap37, 0, !dbg !20
  %vec.needs_grow40 = icmp eq i64 %vec.current_size36, %vec.current_cap37, !dbg !20
  %vec.needs_realloc41 = or i1 %vec.needs_alloc39, %vec.needs_grow40, !dbg !20
  br i1 %vec.needs_realloc41, label %vec.alloc42, label %vec.merge45, !dbg !20

vec.alloc42:                                      ; preds = %vec.merge17
  %6 = mul i64 %vec.current_cap37, 2, !dbg !20
  %vec.new_cap46 = select i1 %vec.needs_alloc39, i64 4, i64 %6, !dbg !20
  %vec.alloc_size47 = mul i64 %vec.new_cap46, 16, !dbg !20
  %vec.new_data48 = call ptr @malloc(i64 %vec.alloc_size47), !dbg !20
  %vec.has_data49 = icmp ne i64 %vec.current_size36, 0, !dbg !20
  br i1 %vec.has_data49, label %vec.copy43, label %vec.no_copy44, !dbg !20

vec.copy43:                                       ; preds = %vec.alloc42
  %vec.copy_size50 = mul i64 %vec.current_size36, 16, !dbg !20
  %7 = call ptr @memcpy(ptr %vec.new_data48, ptr %vec.data38, i64 %vec.copy_size50), !dbg !20
  br label %vec.no_copy44, !dbg !20

vec.no_copy44:                                    ; preds = %vec.copy43, %vec.alloc42
  store ptr %vec.new_data48, ptr %vec.data_ptr33, align 8, !dbg !20
  store i64 %vec.new_cap46, ptr %vec.cap_ptr35, align 4, !dbg !20
  br label %vec.merge45, !dbg !20

vec.merge45:                                      ; preds = %vec.no_copy44, %vec.merge17
  %vec.final_data51 = load ptr, ptr %vec.data_ptr33, align 8, !dbg !20
  %vec.reloaded_size52 = load i64, ptr %vec.size_ptr34, align 4, !dbg !20
  %vec.offset53 = mul i64 %vec.reloaded_size52, 16, !dbg !20
  %vec.element_ptr54 = getelementptr i8, ptr %vec.final_data51, i64 %vec.offset53, !dbg !20
  %vec.temp_struct55 = alloca %Point, align 8, !dbg !20
  store %Point %Point_val32, ptr %vec.temp_struct55, align 8, !dbg !20
  %8 = call ptr @memcpy(ptr %vec.element_ptr54, ptr %vec.temp_struct55, i64 16), !dbg !20
  %vec.new_size56 = add i64 %vec.reloaded_size52, 1, !dbg !20
  store i64 %vec.new_size56, ptr %vec.size_ptr34, align 4, !dbg !20
  %Point_obj57 = alloca %Point, align 8, !dbg !20
  %x_ptr58 = getelementptr inbounds %Point, ptr %Point_obj57, i32 0, i32 0, !dbg !20
  store double 2.000000e+00, ptr %x_ptr58, align 8, !dbg !20
  %y_ptr59 = getelementptr inbounds %Point, ptr %Point_obj57, i32 0, i32 1, !dbg !20
  store double 0.000000e+00, ptr %y_ptr59, align 8, !dbg !20
  %Point_val60 = load %Point, ptr %Point_obj57, align 8, !dbg !20
  %vec.data_ptr61 = getelementptr inbounds { ptr, i64, i64 }, ptr %points, i32 0, i32 0, !dbg !20
  %vec.size_ptr62 = getelementptr inbounds { ptr, i64, i64 }, ptr %points, i32 0, i32 1, !dbg !20
  %vec.cap_ptr63 = getelementptr inbounds { ptr, i64, i64 }, ptr %points, i32 0, i32 2, !dbg !20
  %vec.current_size64 = load i64, ptr %vec.size_ptr62, align 4, !dbg !20
  %vec.current_cap65 = load i64, ptr %vec.cap_ptr63, align 4, !dbg !20
  %vec.data66 = load ptr, ptr %vec.data_ptr61, align 8, !dbg !20
  %vec.needs_alloc67 = icmp eq i64 %vec.current_cap65, 0, !dbg !20
  %vec.needs_grow68 = icmp eq i64 %vec.current_size64, %vec.current_cap65, !dbg !20
  %vec.needs_realloc69 = or i1 %vec.needs_alloc67, %vec.needs_grow68, !dbg !20
  br i1 %vec.needs_realloc69, label %vec.alloc70, label %vec.merge73, !dbg !20

vec.alloc70:                                      ; preds = %vec.merge45
  %9 = mul i64 %vec.current_cap65, 2, !dbg !20
  %vec.new_cap74 = select i1 %vec.needs_alloc67, i64 4, i64 %9, !dbg !20
  %vec.alloc_size75 = mul i64 %vec.new_cap74, 16, !dbg !20
  %vec.new_data76 = call ptr @malloc(i64 %vec.alloc_size75), !dbg !20
  %vec.has_data77 = icmp ne i64 %vec.current_size64, 0, !dbg !20
  br i1 %vec.has_data77, label %vec.copy71, label %vec.no_copy72, !dbg !20

vec.copy71:                                       ; preds = %vec.alloc70
  %vec.copy_size78 = mul i64 %vec.current_size64, 16, !dbg !20
  %10 = call ptr @memcpy(ptr %vec.new_data76, ptr %vec.data66, i64 %vec.copy_size78), !dbg !20
  br label %vec.no_copy72, !dbg !20

vec.no_copy72:                                    ; preds = %vec.copy71, %vec.alloc70
  store ptr %vec.new_data76, ptr %vec.data_ptr61, align 8, !dbg !20
  store i64 %vec.new_cap74, ptr %vec.cap_ptr63, align 4, !dbg !20
  br label %vec.merge73, !dbg !20

vec.merge73:                                      ; preds = %vec.no_copy72, %vec.merge45
  %vec.final_data79 = load ptr, ptr %vec.data_ptr61, align 8, !dbg !20
  %vec.reloaded_size80 = load i64, ptr %vec.size_ptr62, align 4, !dbg !20
  %vec.offset81 = mul i64 %vec.reloaded_size80, 16, !dbg !20
  %vec.element_ptr82 = getelementptr i8, ptr %vec.final_data79, i64 %vec.offset81, !dbg !20
  %vec.temp_struct83 = alloca %Point, align 8, !dbg !20
  store %Point %Point_val60, ptr %vec.temp_struct83, align 8, !dbg !20
  %11 = call ptr @memcpy(ptr %vec.element_ptr82, ptr %vec.temp_struct83, i64 16), !dbg !20
  %vec.new_size84 = add i64 %vec.reloaded_size80, 1, !dbg !20
  store i64 %vec.new_size84, ptr %vec.size_ptr62, align 4, !dbg !20
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !20
  store { ptr, i64 } { ptr @0, i64 56 }, ptr %serialize_temp, align 8, !dbg !20
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !20
  call void @__vyn_println(ptr %serialized_json), !dbg !20
  %serialize_temp85 = alloca { ptr, i64 }, align 8, !dbg !20
  store { ptr, i64 } { ptr @1, i64 0 }, ptr %serialize_temp85, align 8, !dbg !20
  %serialized_json86 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp85, ptr @type_name.1), !dbg !20
  call void @__vyn_println(ptr %serialized_json86), !dbg !20
  br label %for.init, !dbg !20

for.init:                                         ; preds = %vec.merge73
  store i1 true, ptr %__run_once_p, align 1, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %__run_once_p, metadata !11, metadata !DIExpression()), !dbg !22
  br label %for.cond, !dbg !20

for.cond:                                         ; preds = %for.update, %for.init
  %__run_once_p87 = load i1, ptr %__run_once_p, align 1, !dbg !20
  br i1 %__run_once_p87, label %for.body, label %for.exit, !dbg !20

for.body:                                         ; preds = %for.cond
  %vec.size_ptr88 = getelementptr inbounds { ptr, i64, i64 }, ptr %points, i32 0, i32 1, !dbg !20
  %vec.len = load i64, ptr %vec.size_ptr88, align 4, !dbg !20
  store i64 %vec.len, ptr %__len_p, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %__len_p, metadata !13, metadata !DIExpression()), !dbg !22
  br label %for.init89, !dbg !20

for.update:                                       ; preds = %for.exit93
  store i1 false, ptr %__run_once_p, align 1, !dbg !20
  br label %for.cond, !dbg !20

for.exit:                                         ; preds = %for.cond
  %vec.size_ptr120 = getelementptr inbounds { ptr, i64, i64 }, ptr %points, i32 0, i32 1, !dbg !20
  %vec.len121 = load i64, ptr %vec.size_ptr120, align 4, !dbg !20
  store i64 %vec.len121, ptr %count, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %count, metadata !19, metadata !DIExpression()), !dbg !23
  %count122 = load i64, ptr %count, align 4, !dbg !20
  %points_cleanup_load = load { ptr, i64, i64 }, ptr %points, align 8, !dbg !20
  %points_data_ptr = extractvalue { ptr, i64, i64 } %points_cleanup_load, 0, !dbg !20
  %points_null_check = icmp ne ptr %points_data_ptr, null, !dbg !20
  br i1 %points_null_check, label %points_free_block, label %points_continue, !dbg !20

for.init89:                                       ; preds = %for.body
  store i64 0, ptr %__idx_p, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %__idx_p, metadata !14, metadata !DIExpression()), !dbg !22
  br label %for.cond90, !dbg !20

for.cond90:                                       ; preds = %for.update92, %for.init89
  %__idx_p94 = load i64, ptr %__idx_p, align 4, !dbg !20
  %__len_p95 = load i64, ptr %__len_p, align 4, !dbg !20
  %icmpslttmp = icmp slt i64 %__idx_p94, %__len_p95, !dbg !20
  br i1 %icmpslttmp, label %for.body91, label %for.exit93, !dbg !20

for.body91:                                       ; preds = %for.cond90
  %__idx_p96 = load i64, ptr %__idx_p, align 4, !dbg !20
  %vec.data_ptr97 = getelementptr inbounds { ptr, i64, i64 }, ptr %points, i32 0, i32 0, !dbg !20
  %vec.size_ptr98 = getelementptr inbounds { ptr, i64, i64 }, ptr %points, i32 0, i32 1, !dbg !20
  %vec.data99 = load ptr, ptr %vec.data_ptr97, align 8, !dbg !20
  %vec.size = load i64, ptr %vec.size_ptr98, align 4, !dbg !20
  %vec.offset100 = mul i64 %__idx_p96, 16, !dbg !20
  %vec.element_ptr101 = getelementptr i8, ptr %vec.data99, i64 %vec.offset100, !dbg !20
  %vec.element_struct = load %Point, ptr %vec.element_ptr101, align 8, !dbg !20
  store %Point %vec.element_struct, ptr %p, align 8, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %p, metadata !15, metadata !DIExpression()), !dbg !22
  %p102 = load %Point, ptr %p, align 8, !dbg !20
  %temp_struct = alloca %Point, align 8, !dbg !20
  store %Point %p102, ptr %temp_struct, align 8, !dbg !20
  %x_ptr103 = getelementptr inbounds %Point, ptr %temp_struct, i32 0, i32 0, !dbg !20
  %x_val = load double, ptr %x_ptr103, align 8, !dbg !20
  %p104 = load %Point, ptr %p, align 8, !dbg !20
  %temp_struct105 = alloca %Point, align 8, !dbg !20
  store %Point %p104, ptr %temp_struct105, align 8, !dbg !20
  %x_ptr106 = getelementptr inbounds %Point, ptr %temp_struct105, i32 0, i32 0, !dbg !20
  %x_val107 = load double, ptr %x_ptr106, align 8, !dbg !20
  %fmultmp = fmul double %x_val, %x_val107, !dbg !20
  %p108 = load %Point, ptr %p, align 8, !dbg !20
  %temp_struct109 = alloca %Point, align 8, !dbg !20
  store %Point %p108, ptr %temp_struct109, align 8, !dbg !20
  %y_ptr110 = getelementptr inbounds %Point, ptr %temp_struct109, i32 0, i32 1, !dbg !20
  %y_val = load double, ptr %y_ptr110, align 8, !dbg !20
  %p111 = load %Point, ptr %p, align 8, !dbg !20
  %temp_struct112 = alloca %Point, align 8, !dbg !20
  store %Point %p111, ptr %temp_struct112, align 8, !dbg !20
  %y_ptr113 = getelementptr inbounds %Point, ptr %temp_struct112, i32 0, i32 1, !dbg !20
  %y_val114 = load double, ptr %y_ptr113, align 8, !dbg !20
  %fmultmp115 = fmul double %y_val, %y_val114, !dbg !20
  %faddtmp = fadd double %fmultmp, %fmultmp115, !dbg !20
  store double %faddtmp, ptr %distance_sq, align 8, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %distance_sq, metadata !17, metadata !DIExpression()), !dbg !24
  %distance_sq116 = load double, ptr %distance_sq, align 8, !dbg !20
  %serialize_temp117 = alloca double, align 8, !dbg !20
  store double %distance_sq116, ptr %serialize_temp117, align 8, !dbg !20
  %serialized_json118 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp117, ptr @type_name.2), !dbg !20
  call void @__vyn_println(ptr %serialized_json118), !dbg !20
  br label %for.update92, !dbg !20

for.update92:                                     ; preds = %for.body91
  %__idx_p119 = load i64, ptr %__idx_p, align 4, !dbg !20
  %addtmp = add i64 %__idx_p119, 1, !dbg !20
  store i64 %addtmp, ptr %__idx_p, align 4, !dbg !20
  br label %for.cond90, !dbg !20

for.exit93:                                       ; preds = %for.cond90
  br label %for.update, !dbg !20

points_free_block:                                ; preds = %for.exit
  call void @free(ptr %points_data_ptr), !dbg !20
  br label %points_continue, !dbg !20

points_continue:                                  ; preds = %points_free_block, %for.exit
  ret i64 %count122, !dbg !20
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @malloc(i64)

declare ptr @memcpy(ptr, ptr, i64)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)

declare void @free(ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "vec_point_distance.vyn.ll", directory: "/home/rick/Projects/Vyn/examples")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 11, type: !5, scopeLine: 11, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11, !13, !14, !15, !17, !19}
!9 = !DILocalVariable(name: "points", scope: !4, file: !1, line: 12, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_{ ptr, i64, i64 }", scope: !1, file: !1, size: 192, align: 8)
!11 = !DILocalVariable(name: "__run_once_p", scope: !4, file: !1, line: 25, type: !12)
!12 = !DIBasicType(name: "bool", size: 1, encoding: DW_ATE_boolean)
!13 = !DILocalVariable(name: "__len_p", scope: !4, file: !1, line: 25, type: !7)
!14 = !DILocalVariable(name: "__idx_p", scope: !4, file: !1, line: 25, type: !7)
!15 = !DILocalVariable(name: "p", scope: !4, file: !1, line: 25, type: !16)
!16 = !DICompositeType(tag: DW_TAG_structure_type, name: "Point", scope: !1, file: !1, size: 128, align: 8)
!17 = !DILocalVariable(name: "distance_sq", scope: !4, file: !1, line: 26, type: !18)
!18 = !DIBasicType(name: "f64", size: 64, encoding: DW_ATE_float)
!19 = !DILocalVariable(name: "count", scope: !4, file: !1, line: 30, type: !7)
!20 = !DILocation(line: 11, column: 1, scope: !4)
!21 = !DILocation(line: 12, column: 5, scope: !4)
!22 = !DILocation(line: 25, column: 10, scope: !4)
!23 = !DILocation(line: 30, column: 1, scope: !4)
!24 = !DILocation(line: 26, column: 1, scope: !4)
