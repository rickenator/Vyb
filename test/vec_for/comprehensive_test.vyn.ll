; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() !dbg !4 {
entry:
  %x370 = alloca i64, align 8, !dbg !36
  %__idx_x358 = alloca i64, align 8, !dbg !36
  %__len_x352 = alloca i64, align 8, !dbg !36
  %__run_once_x348 = alloca i1, align 1, !dbg !36
  %skip_pos = alloca i64, align 8, !dbg !36
  %sum4 = alloca i64, align 8, !dbg !36
  %v4 = alloca { ptr, i64, i64 }, align 8, !dbg !36
  %x233 = alloca i64, align 8, !dbg !36
  %__idx_x221 = alloca i64, align 8, !dbg !36
  %__len_x215 = alloca i64, align 8, !dbg !36
  %__run_once_x211 = alloca i1, align 1, !dbg !36
  %sum3 = alloca i64, align 8, !dbg !36
  %v3 = alloca { ptr, i64, i64 }, align 8, !dbg !36
  %x99 = alloca i64, align 8, !dbg !36
  %__idx_x87 = alloca i64, align 8, !dbg !36
  %__len_x81 = alloca i64, align 8, !dbg !36
  %__run_once_x77 = alloca i1, align 1, !dbg !36
  %sum2 = alloca i64, align 8, !dbg !36
  %v2 = alloca { ptr, i64, i64 }, align 8, !dbg !36
  %x = alloca i64, align 8, !dbg !36
  %__idx_x = alloca i64, align 8, !dbg !36
  %__len_x = alloca i64, align 8, !dbg !36
  %__run_once_x = alloca i1, align 1, !dbg !36
  %sum1 = alloca i64, align 8, !dbg !36
  %v1 = alloca { ptr, i64, i64 }, align 8, !dbg !36
  %vec.new = alloca { ptr, i64, i64 }, align 8, !dbg !36
  %vec.ptr_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 0, !dbg !36
  store ptr null, ptr %vec.ptr_field, align 8, !dbg !36
  %vec.size_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 1, !dbg !36
  store i64 0, ptr %vec.size_field, align 4, !dbg !36
  %vec.cap_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 2, !dbg !36
  store i64 0, ptr %vec.cap_field, align 4, !dbg !36
  %vec.new.value = load { ptr, i64, i64 }, ptr %vec.new, align 8, !dbg !36
  store { ptr, i64, i64 } %vec.new.value, ptr %v1, align 8, !dbg !36
  call void @llvm.dbg.declare(metadata ptr %v1, metadata !9, metadata !DIExpression()), !dbg !37
  %vec.data_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %v1, i32 0, i32 0, !dbg !36
  %vec.size_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %v1, i32 0, i32 1, !dbg !36
  %vec.cap_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %v1, i32 0, i32 2, !dbg !36
  %vec.current_size = load i64, ptr %vec.size_ptr, align 4, !dbg !36
  %vec.current_cap = load i64, ptr %vec.cap_ptr, align 4, !dbg !36
  %vec.data = load ptr, ptr %vec.data_ptr, align 8, !dbg !36
  %vec.needs_alloc = icmp eq i64 %vec.current_cap, 0, !dbg !36
  %vec.needs_grow = icmp eq i64 %vec.current_size, %vec.current_cap, !dbg !36
  %vec.needs_realloc = or i1 %vec.needs_alloc, %vec.needs_grow, !dbg !36
  br i1 %vec.needs_realloc, label %vec.alloc, label %vec.merge, !dbg !36

vec.alloc:                                        ; preds = %entry
  %0 = mul i64 %vec.current_cap, 2, !dbg !36
  %vec.new_cap = select i1 %vec.needs_alloc, i64 4, i64 %0, !dbg !36
  %vec.alloc_size = mul i64 %vec.new_cap, 8, !dbg !36
  %vec.new_data = call ptr @malloc(i64 %vec.alloc_size), !dbg !36
  %vec.has_data = icmp ne i64 %vec.current_size, 0, !dbg !36
  br i1 %vec.has_data, label %vec.copy, label %vec.no_copy, !dbg !36

vec.copy:                                         ; preds = %vec.alloc
  %vec.copy_size = mul i64 %vec.current_size, 8, !dbg !36
  %1 = call ptr @memcpy(ptr %vec.new_data, ptr %vec.data, i64 %vec.copy_size), !dbg !36
  br label %vec.no_copy, !dbg !36

vec.no_copy:                                      ; preds = %vec.copy, %vec.alloc
  store ptr %vec.new_data, ptr %vec.data_ptr, align 8, !dbg !36
  store i64 %vec.new_cap, ptr %vec.cap_ptr, align 4, !dbg !36
  br label %vec.merge, !dbg !36

vec.merge:                                        ; preds = %vec.no_copy, %entry
  %vec.final_data = load ptr, ptr %vec.data_ptr, align 8, !dbg !36
  %vec.reloaded_size = load i64, ptr %vec.size_ptr, align 4, !dbg !36
  %vec.offset = mul i64 %vec.reloaded_size, 8, !dbg !36
  %vec.element_ptr = getelementptr i8, ptr %vec.final_data, i64 %vec.offset, !dbg !36
  store i64 1, ptr %vec.element_ptr, align 4, !dbg !36
  %vec.new_size = add i64 %vec.reloaded_size, 1, !dbg !36
  store i64 %vec.new_size, ptr %vec.size_ptr, align 4, !dbg !36
  %vec.data_ptr1 = getelementptr inbounds { ptr, i64, i64 }, ptr %v1, i32 0, i32 0, !dbg !36
  %vec.size_ptr2 = getelementptr inbounds { ptr, i64, i64 }, ptr %v1, i32 0, i32 1, !dbg !36
  %vec.cap_ptr3 = getelementptr inbounds { ptr, i64, i64 }, ptr %v1, i32 0, i32 2, !dbg !36
  %vec.current_size4 = load i64, ptr %vec.size_ptr2, align 4, !dbg !36
  %vec.current_cap5 = load i64, ptr %vec.cap_ptr3, align 4, !dbg !36
  %vec.data6 = load ptr, ptr %vec.data_ptr1, align 8, !dbg !36
  %vec.needs_alloc7 = icmp eq i64 %vec.current_cap5, 0, !dbg !36
  %vec.needs_grow8 = icmp eq i64 %vec.current_size4, %vec.current_cap5, !dbg !36
  %vec.needs_realloc9 = or i1 %vec.needs_alloc7, %vec.needs_grow8, !dbg !36
  br i1 %vec.needs_realloc9, label %vec.alloc10, label %vec.merge13, !dbg !36

vec.alloc10:                                      ; preds = %vec.merge
  %2 = mul i64 %vec.current_cap5, 2, !dbg !36
  %vec.new_cap14 = select i1 %vec.needs_alloc7, i64 4, i64 %2, !dbg !36
  %vec.alloc_size15 = mul i64 %vec.new_cap14, 8, !dbg !36
  %vec.new_data16 = call ptr @malloc(i64 %vec.alloc_size15), !dbg !36
  %vec.has_data17 = icmp ne i64 %vec.current_size4, 0, !dbg !36
  br i1 %vec.has_data17, label %vec.copy11, label %vec.no_copy12, !dbg !36

vec.copy11:                                       ; preds = %vec.alloc10
  %vec.copy_size18 = mul i64 %vec.current_size4, 8, !dbg !36
  %3 = call ptr @memcpy(ptr %vec.new_data16, ptr %vec.data6, i64 %vec.copy_size18), !dbg !36
  br label %vec.no_copy12, !dbg !36

vec.no_copy12:                                    ; preds = %vec.copy11, %vec.alloc10
  store ptr %vec.new_data16, ptr %vec.data_ptr1, align 8, !dbg !36
  store i64 %vec.new_cap14, ptr %vec.cap_ptr3, align 4, !dbg !36
  br label %vec.merge13, !dbg !36

vec.merge13:                                      ; preds = %vec.no_copy12, %vec.merge
  %vec.final_data19 = load ptr, ptr %vec.data_ptr1, align 8, !dbg !36
  %vec.reloaded_size20 = load i64, ptr %vec.size_ptr2, align 4, !dbg !36
  %vec.offset21 = mul i64 %vec.reloaded_size20, 8, !dbg !36
  %vec.element_ptr22 = getelementptr i8, ptr %vec.final_data19, i64 %vec.offset21, !dbg !36
  store i64 2, ptr %vec.element_ptr22, align 4, !dbg !36
  %vec.new_size23 = add i64 %vec.reloaded_size20, 1, !dbg !36
  store i64 %vec.new_size23, ptr %vec.size_ptr2, align 4, !dbg !36
  %vec.data_ptr24 = getelementptr inbounds { ptr, i64, i64 }, ptr %v1, i32 0, i32 0, !dbg !36
  %vec.size_ptr25 = getelementptr inbounds { ptr, i64, i64 }, ptr %v1, i32 0, i32 1, !dbg !36
  %vec.cap_ptr26 = getelementptr inbounds { ptr, i64, i64 }, ptr %v1, i32 0, i32 2, !dbg !36
  %vec.current_size27 = load i64, ptr %vec.size_ptr25, align 4, !dbg !36
  %vec.current_cap28 = load i64, ptr %vec.cap_ptr26, align 4, !dbg !36
  %vec.data29 = load ptr, ptr %vec.data_ptr24, align 8, !dbg !36
  %vec.needs_alloc30 = icmp eq i64 %vec.current_cap28, 0, !dbg !36
  %vec.needs_grow31 = icmp eq i64 %vec.current_size27, %vec.current_cap28, !dbg !36
  %vec.needs_realloc32 = or i1 %vec.needs_alloc30, %vec.needs_grow31, !dbg !36
  br i1 %vec.needs_realloc32, label %vec.alloc33, label %vec.merge36, !dbg !36

vec.alloc33:                                      ; preds = %vec.merge13
  %4 = mul i64 %vec.current_cap28, 2, !dbg !36
  %vec.new_cap37 = select i1 %vec.needs_alloc30, i64 4, i64 %4, !dbg !36
  %vec.alloc_size38 = mul i64 %vec.new_cap37, 8, !dbg !36
  %vec.new_data39 = call ptr @malloc(i64 %vec.alloc_size38), !dbg !36
  %vec.has_data40 = icmp ne i64 %vec.current_size27, 0, !dbg !36
  br i1 %vec.has_data40, label %vec.copy34, label %vec.no_copy35, !dbg !36

vec.copy34:                                       ; preds = %vec.alloc33
  %vec.copy_size41 = mul i64 %vec.current_size27, 8, !dbg !36
  %5 = call ptr @memcpy(ptr %vec.new_data39, ptr %vec.data29, i64 %vec.copy_size41), !dbg !36
  br label %vec.no_copy35, !dbg !36

vec.no_copy35:                                    ; preds = %vec.copy34, %vec.alloc33
  store ptr %vec.new_data39, ptr %vec.data_ptr24, align 8, !dbg !36
  store i64 %vec.new_cap37, ptr %vec.cap_ptr26, align 4, !dbg !36
  br label %vec.merge36, !dbg !36

vec.merge36:                                      ; preds = %vec.no_copy35, %vec.merge13
  %vec.final_data42 = load ptr, ptr %vec.data_ptr24, align 8, !dbg !36
  %vec.reloaded_size43 = load i64, ptr %vec.size_ptr25, align 4, !dbg !36
  %vec.offset44 = mul i64 %vec.reloaded_size43, 8, !dbg !36
  %vec.element_ptr45 = getelementptr i8, ptr %vec.final_data42, i64 %vec.offset44, !dbg !36
  store i64 3, ptr %vec.element_ptr45, align 4, !dbg !36
  %vec.new_size46 = add i64 %vec.reloaded_size43, 1, !dbg !36
  store i64 %vec.new_size46, ptr %vec.size_ptr25, align 4, !dbg !36
  store i64 0, ptr %sum1, align 4, !dbg !36
  call void @llvm.dbg.declare(metadata ptr %sum1, metadata !11, metadata !DIExpression()), !dbg !38
  br label %for.init, !dbg !36

for.init:                                         ; preds = %vec.merge36
  store i1 true, ptr %__run_once_x, align 1, !dbg !36
  call void @llvm.dbg.declare(metadata ptr %__run_once_x, metadata !12, metadata !DIExpression()), !dbg !39
  br label %for.cond, !dbg !36

for.cond:                                         ; preds = %for.update, %for.init
  %__run_once_x47 = load i1, ptr %__run_once_x, align 1, !dbg !36
  br i1 %__run_once_x47, label %for.body, label %for.exit, !dbg !36

for.body:                                         ; preds = %for.cond
  %vec.size_ptr48 = getelementptr inbounds { ptr, i64, i64 }, ptr %v1, i32 0, i32 1, !dbg !36
  %vec.len = load i64, ptr %vec.size_ptr48, align 4, !dbg !36
  store i64 %vec.len, ptr %__len_x, align 4, !dbg !36
  call void @llvm.dbg.declare(metadata ptr %__len_x, metadata !14, metadata !DIExpression()), !dbg !39
  br label %for.init49, !dbg !36

for.update:                                       ; preds = %for.exit53
  store i1 false, ptr %__run_once_x, align 1, !dbg !36
  br label %for.cond, !dbg !36

for.exit:                                         ; preds = %for.cond
  %sum166 = load i64, ptr %sum1, align 4, !dbg !36
  %icmpneqtmp = icmp ne i64 %sum166, 6, !dbg !36
  br i1 %icmpneqtmp, label %then, label %ifcont, !dbg !36

for.init49:                                       ; preds = %for.body
  store i64 0, ptr %__idx_x, align 4, !dbg !36
  call void @llvm.dbg.declare(metadata ptr %__idx_x, metadata !15, metadata !DIExpression()), !dbg !39
  br label %for.cond50, !dbg !36

for.cond50:                                       ; preds = %for.update52, %for.init49
  %__idx_x54 = load i64, ptr %__idx_x, align 4, !dbg !36
  %__len_x55 = load i64, ptr %__len_x, align 4, !dbg !36
  %icmpslttmp = icmp slt i64 %__idx_x54, %__len_x55, !dbg !36
  br i1 %icmpslttmp, label %for.body51, label %for.exit53, !dbg !36

for.body51:                                       ; preds = %for.cond50
  %__idx_x56 = load i64, ptr %__idx_x, align 4, !dbg !36
  %vec.data_ptr57 = getelementptr inbounds { ptr, i64, i64 }, ptr %v1, i32 0, i32 0, !dbg !36
  %vec.size_ptr58 = getelementptr inbounds { ptr, i64, i64 }, ptr %v1, i32 0, i32 1, !dbg !36
  %vec.data59 = load ptr, ptr %vec.data_ptr57, align 8, !dbg !36
  %vec.size = load i64, ptr %vec.size_ptr58, align 4, !dbg !36
  %vec.offset60 = mul i64 %__idx_x56, 8, !dbg !36
  %vec.element_ptr61 = getelementptr i8, ptr %vec.data59, i64 %vec.offset60, !dbg !36
  %vec.element = load i64, ptr %vec.element_ptr61, align 4, !dbg !36
  store i64 %vec.element, ptr %x, align 4, !dbg !36
  call void @llvm.dbg.declare(metadata ptr %x, metadata !16, metadata !DIExpression()), !dbg !39
  %sum162 = load i64, ptr %sum1, align 4, !dbg !36
  %x63 = load i64, ptr %x, align 4, !dbg !36
  %addtmp = add i64 %sum162, %x63, !dbg !36
  store i64 %addtmp, ptr %sum1, align 4, !dbg !36
  br label %for.update52, !dbg !36

for.update52:                                     ; preds = %for.body51
  %__idx_x64 = load i64, ptr %__idx_x, align 4, !dbg !36
  %addtmp65 = add i64 %__idx_x64, 1, !dbg !36
  store i64 %addtmp65, ptr %__idx_x, align 4, !dbg !36
  br label %for.cond50, !dbg !36

for.exit53:                                       ; preds = %for.cond50
  br label %for.update, !dbg !36

then:                                             ; preds = %for.exit
  ret i64 1, !dbg !36

ifcont:                                           ; preds = %for.exit
  %vec.new67 = alloca { ptr, i64, i64 }, align 8, !dbg !36
  %vec.ptr_field68 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new67, i32 0, i32 0, !dbg !36
  store ptr null, ptr %vec.ptr_field68, align 8, !dbg !36
  %vec.size_field69 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new67, i32 0, i32 1, !dbg !36
  store i64 0, ptr %vec.size_field69, align 4, !dbg !36
  %vec.cap_field70 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new67, i32 0, i32 2, !dbg !36
  store i64 0, ptr %vec.cap_field70, align 4, !dbg !36
  %vec.new.value71 = load { ptr, i64, i64 }, ptr %vec.new67, align 8, !dbg !36
  store { ptr, i64, i64 } %vec.new.value71, ptr %v2, align 8, !dbg !36
  call void @llvm.dbg.declare(metadata ptr %v2, metadata !17, metadata !DIExpression()), !dbg !40
  store i64 0, ptr %sum2, align 4, !dbg !36
  call void @llvm.dbg.declare(metadata ptr %sum2, metadata !18, metadata !DIExpression()), !dbg !41
  br label %for.init72, !dbg !36

for.init72:                                       ; preds = %ifcont
  store i1 true, ptr %__run_once_x77, align 1, !dbg !36
  call void @llvm.dbg.declare(metadata ptr %__run_once_x77, metadata !19, metadata !DIExpression()), !dbg !42
  br label %for.cond73, !dbg !36

for.cond73:                                       ; preds = %for.update75, %for.init72
  %__run_once_x78 = load i1, ptr %__run_once_x77, align 1, !dbg !36
  br i1 %__run_once_x78, label %for.body74, label %for.exit76, !dbg !36

for.body74:                                       ; preds = %for.cond73
  %vec.size_ptr79 = getelementptr inbounds { ptr, i64, i64 }, ptr %v2, i32 0, i32 1, !dbg !36
  %vec.len80 = load i64, ptr %vec.size_ptr79, align 4, !dbg !36
  store i64 %vec.len80, ptr %__len_x81, align 4, !dbg !36
  call void @llvm.dbg.declare(metadata ptr %__len_x81, metadata !20, metadata !DIExpression()), !dbg !42
  br label %for.init82, !dbg !36

for.update75:                                     ; preds = %for.exit86
  store i1 false, ptr %__run_once_x77, align 1, !dbg !36
  br label %for.cond73, !dbg !36

for.exit76:                                       ; preds = %for.cond73
  %sum2105 = load i64, ptr %sum2, align 4, !dbg !36
  %icmpneqtmp106 = icmp ne i64 %sum2105, 0, !dbg !36
  br i1 %icmpneqtmp106, label %then107, label %ifcont108, !dbg !36

for.init82:                                       ; preds = %for.body74
  store i64 0, ptr %__idx_x87, align 4, !dbg !36
  call void @llvm.dbg.declare(metadata ptr %__idx_x87, metadata !21, metadata !DIExpression()), !dbg !42
  br label %for.cond83, !dbg !36

for.cond83:                                       ; preds = %for.update85, %for.init82
  %__idx_x88 = load i64, ptr %__idx_x87, align 4, !dbg !36
  %__len_x89 = load i64, ptr %__len_x81, align 4, !dbg !36
  %icmpslttmp90 = icmp slt i64 %__idx_x88, %__len_x89, !dbg !36
  br i1 %icmpslttmp90, label %for.body84, label %for.exit86, !dbg !36

for.body84:                                       ; preds = %for.cond83
  %__idx_x91 = load i64, ptr %__idx_x87, align 4, !dbg !36
  %vec.data_ptr92 = getelementptr inbounds { ptr, i64, i64 }, ptr %v2, i32 0, i32 0, !dbg !36
  %vec.size_ptr93 = getelementptr inbounds { ptr, i64, i64 }, ptr %v2, i32 0, i32 1, !dbg !36
  %vec.data94 = load ptr, ptr %vec.data_ptr92, align 8, !dbg !36
  %vec.size95 = load i64, ptr %vec.size_ptr93, align 4, !dbg !36
  %vec.offset96 = mul i64 %__idx_x91, 8, !dbg !36
  %vec.element_ptr97 = getelementptr i8, ptr %vec.data94, i64 %vec.offset96, !dbg !36
  %vec.element98 = load i64, ptr %vec.element_ptr97, align 4, !dbg !36
  store i64 %vec.element98, ptr %x99, align 4, !dbg !36
  call void @llvm.dbg.declare(metadata ptr %x99, metadata !22, metadata !DIExpression()), !dbg !42
  %sum2100 = load i64, ptr %sum2, align 4, !dbg !36
  %x101 = load i64, ptr %x99, align 4, !dbg !36
  %addtmp102 = add i64 %sum2100, %x101, !dbg !36
  store i64 %addtmp102, ptr %sum2, align 4, !dbg !36
  br label %for.update85, !dbg !36

for.update85:                                     ; preds = %for.body84
  %__idx_x103 = load i64, ptr %__idx_x87, align 4, !dbg !36
  %addtmp104 = add i64 %__idx_x103, 1, !dbg !36
  store i64 %addtmp104, ptr %__idx_x87, align 4, !dbg !36
  br label %for.cond83, !dbg !36

for.exit86:                                       ; preds = %for.cond83
  br label %for.update75, !dbg !36

then107:                                          ; preds = %for.exit76
  ret i64 2, !dbg !36

ifcont108:                                        ; preds = %for.exit76
  %vec.new109 = alloca { ptr, i64, i64 }, align 8, !dbg !36
  %vec.ptr_field110 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new109, i32 0, i32 0, !dbg !36
  store ptr null, ptr %vec.ptr_field110, align 8, !dbg !36
  %vec.size_field111 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new109, i32 0, i32 1, !dbg !36
  store i64 0, ptr %vec.size_field111, align 4, !dbg !36
  %vec.cap_field112 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new109, i32 0, i32 2, !dbg !36
  store i64 0, ptr %vec.cap_field112, align 4, !dbg !36
  %vec.new.value113 = load { ptr, i64, i64 }, ptr %vec.new109, align 8, !dbg !36
  store { ptr, i64, i64 } %vec.new.value113, ptr %v3, align 8, !dbg !36
  call void @llvm.dbg.declare(metadata ptr %v3, metadata !23, metadata !DIExpression()), !dbg !43
  %vec.data_ptr114 = getelementptr inbounds { ptr, i64, i64 }, ptr %v3, i32 0, i32 0, !dbg !36
  %vec.size_ptr115 = getelementptr inbounds { ptr, i64, i64 }, ptr %v3, i32 0, i32 1, !dbg !36
  %vec.cap_ptr116 = getelementptr inbounds { ptr, i64, i64 }, ptr %v3, i32 0, i32 2, !dbg !36
  %vec.current_size117 = load i64, ptr %vec.size_ptr115, align 4, !dbg !36
  %vec.current_cap118 = load i64, ptr %vec.cap_ptr116, align 4, !dbg !36
  %vec.data119 = load ptr, ptr %vec.data_ptr114, align 8, !dbg !36
  %vec.needs_alloc120 = icmp eq i64 %vec.current_cap118, 0, !dbg !36
  %vec.needs_grow121 = icmp eq i64 %vec.current_size117, %vec.current_cap118, !dbg !36
  %vec.needs_realloc122 = or i1 %vec.needs_alloc120, %vec.needs_grow121, !dbg !36
  br i1 %vec.needs_realloc122, label %vec.alloc123, label %vec.merge126, !dbg !36

vec.alloc123:                                     ; preds = %ifcont108
  %6 = mul i64 %vec.current_cap118, 2, !dbg !36
  %vec.new_cap127 = select i1 %vec.needs_alloc120, i64 4, i64 %6, !dbg !36
  %vec.alloc_size128 = mul i64 %vec.new_cap127, 8, !dbg !36
  %vec.new_data129 = call ptr @malloc(i64 %vec.alloc_size128), !dbg !36
  %vec.has_data130 = icmp ne i64 %vec.current_size117, 0, !dbg !36
  br i1 %vec.has_data130, label %vec.copy124, label %vec.no_copy125, !dbg !36

vec.copy124:                                      ; preds = %vec.alloc123
  %vec.copy_size131 = mul i64 %vec.current_size117, 8, !dbg !36
  %7 = call ptr @memcpy(ptr %vec.new_data129, ptr %vec.data119, i64 %vec.copy_size131), !dbg !36
  br label %vec.no_copy125, !dbg !36

vec.no_copy125:                                   ; preds = %vec.copy124, %vec.alloc123
  store ptr %vec.new_data129, ptr %vec.data_ptr114, align 8, !dbg !36
  store i64 %vec.new_cap127, ptr %vec.cap_ptr116, align 4, !dbg !36
  br label %vec.merge126, !dbg !36

vec.merge126:                                     ; preds = %vec.no_copy125, %ifcont108
  %vec.final_data132 = load ptr, ptr %vec.data_ptr114, align 8, !dbg !36
  %vec.reloaded_size133 = load i64, ptr %vec.size_ptr115, align 4, !dbg !36
  %vec.offset134 = mul i64 %vec.reloaded_size133, 8, !dbg !36
  %vec.element_ptr135 = getelementptr i8, ptr %vec.final_data132, i64 %vec.offset134, !dbg !36
  store i64 1, ptr %vec.element_ptr135, align 4, !dbg !36
  %vec.new_size136 = add i64 %vec.reloaded_size133, 1, !dbg !36
  store i64 %vec.new_size136, ptr %vec.size_ptr115, align 4, !dbg !36
  %vec.data_ptr137 = getelementptr inbounds { ptr, i64, i64 }, ptr %v3, i32 0, i32 0, !dbg !36
  %vec.size_ptr138 = getelementptr inbounds { ptr, i64, i64 }, ptr %v3, i32 0, i32 1, !dbg !36
  %vec.cap_ptr139 = getelementptr inbounds { ptr, i64, i64 }, ptr %v3, i32 0, i32 2, !dbg !36
  %vec.current_size140 = load i64, ptr %vec.size_ptr138, align 4, !dbg !36
  %vec.current_cap141 = load i64, ptr %vec.cap_ptr139, align 4, !dbg !36
  %vec.data142 = load ptr, ptr %vec.data_ptr137, align 8, !dbg !36
  %vec.needs_alloc143 = icmp eq i64 %vec.current_cap141, 0, !dbg !36
  %vec.needs_grow144 = icmp eq i64 %vec.current_size140, %vec.current_cap141, !dbg !36
  %vec.needs_realloc145 = or i1 %vec.needs_alloc143, %vec.needs_grow144, !dbg !36
  br i1 %vec.needs_realloc145, label %vec.alloc146, label %vec.merge149, !dbg !36

vec.alloc146:                                     ; preds = %vec.merge126
  %8 = mul i64 %vec.current_cap141, 2, !dbg !36
  %vec.new_cap150 = select i1 %vec.needs_alloc143, i64 4, i64 %8, !dbg !36
  %vec.alloc_size151 = mul i64 %vec.new_cap150, 8, !dbg !36
  %vec.new_data152 = call ptr @malloc(i64 %vec.alloc_size151), !dbg !36
  %vec.has_data153 = icmp ne i64 %vec.current_size140, 0, !dbg !36
  br i1 %vec.has_data153, label %vec.copy147, label %vec.no_copy148, !dbg !36

vec.copy147:                                      ; preds = %vec.alloc146
  %vec.copy_size154 = mul i64 %vec.current_size140, 8, !dbg !36
  %9 = call ptr @memcpy(ptr %vec.new_data152, ptr %vec.data142, i64 %vec.copy_size154), !dbg !36
  br label %vec.no_copy148, !dbg !36

vec.no_copy148:                                   ; preds = %vec.copy147, %vec.alloc146
  store ptr %vec.new_data152, ptr %vec.data_ptr137, align 8, !dbg !36
  store i64 %vec.new_cap150, ptr %vec.cap_ptr139, align 4, !dbg !36
  br label %vec.merge149, !dbg !36

vec.merge149:                                     ; preds = %vec.no_copy148, %vec.merge126
  %vec.final_data155 = load ptr, ptr %vec.data_ptr137, align 8, !dbg !36
  %vec.reloaded_size156 = load i64, ptr %vec.size_ptr138, align 4, !dbg !36
  %vec.offset157 = mul i64 %vec.reloaded_size156, 8, !dbg !36
  %vec.element_ptr158 = getelementptr i8, ptr %vec.final_data155, i64 %vec.offset157, !dbg !36
  store i64 2, ptr %vec.element_ptr158, align 4, !dbg !36
  %vec.new_size159 = add i64 %vec.reloaded_size156, 1, !dbg !36
  store i64 %vec.new_size159, ptr %vec.size_ptr138, align 4, !dbg !36
  %vec.data_ptr160 = getelementptr inbounds { ptr, i64, i64 }, ptr %v3, i32 0, i32 0, !dbg !36
  %vec.size_ptr161 = getelementptr inbounds { ptr, i64, i64 }, ptr %v3, i32 0, i32 1, !dbg !36
  %vec.cap_ptr162 = getelementptr inbounds { ptr, i64, i64 }, ptr %v3, i32 0, i32 2, !dbg !36
  %vec.current_size163 = load i64, ptr %vec.size_ptr161, align 4, !dbg !36
  %vec.current_cap164 = load i64, ptr %vec.cap_ptr162, align 4, !dbg !36
  %vec.data165 = load ptr, ptr %vec.data_ptr160, align 8, !dbg !36
  %vec.needs_alloc166 = icmp eq i64 %vec.current_cap164, 0, !dbg !36
  %vec.needs_grow167 = icmp eq i64 %vec.current_size163, %vec.current_cap164, !dbg !36
  %vec.needs_realloc168 = or i1 %vec.needs_alloc166, %vec.needs_grow167, !dbg !36
  br i1 %vec.needs_realloc168, label %vec.alloc169, label %vec.merge172, !dbg !36

vec.alloc169:                                     ; preds = %vec.merge149
  %10 = mul i64 %vec.current_cap164, 2, !dbg !36
  %vec.new_cap173 = select i1 %vec.needs_alloc166, i64 4, i64 %10, !dbg !36
  %vec.alloc_size174 = mul i64 %vec.new_cap173, 8, !dbg !36
  %vec.new_data175 = call ptr @malloc(i64 %vec.alloc_size174), !dbg !36
  %vec.has_data176 = icmp ne i64 %vec.current_size163, 0, !dbg !36
  br i1 %vec.has_data176, label %vec.copy170, label %vec.no_copy171, !dbg !36

vec.copy170:                                      ; preds = %vec.alloc169
  %vec.copy_size177 = mul i64 %vec.current_size163, 8, !dbg !36
  %11 = call ptr @memcpy(ptr %vec.new_data175, ptr %vec.data165, i64 %vec.copy_size177), !dbg !36
  br label %vec.no_copy171, !dbg !36

vec.no_copy171:                                   ; preds = %vec.copy170, %vec.alloc169
  store ptr %vec.new_data175, ptr %vec.data_ptr160, align 8, !dbg !36
  store i64 %vec.new_cap173, ptr %vec.cap_ptr162, align 4, !dbg !36
  br label %vec.merge172, !dbg !36

vec.merge172:                                     ; preds = %vec.no_copy171, %vec.merge149
  %vec.final_data178 = load ptr, ptr %vec.data_ptr160, align 8, !dbg !36
  %vec.reloaded_size179 = load i64, ptr %vec.size_ptr161, align 4, !dbg !36
  %vec.offset180 = mul i64 %vec.reloaded_size179, 8, !dbg !36
  %vec.element_ptr181 = getelementptr i8, ptr %vec.final_data178, i64 %vec.offset180, !dbg !36
  store i64 3, ptr %vec.element_ptr181, align 4, !dbg !36
  %vec.new_size182 = add i64 %vec.reloaded_size179, 1, !dbg !36
  store i64 %vec.new_size182, ptr %vec.size_ptr161, align 4, !dbg !36
  %vec.data_ptr183 = getelementptr inbounds { ptr, i64, i64 }, ptr %v3, i32 0, i32 0, !dbg !36
  %vec.size_ptr184 = getelementptr inbounds { ptr, i64, i64 }, ptr %v3, i32 0, i32 1, !dbg !36
  %vec.cap_ptr185 = getelementptr inbounds { ptr, i64, i64 }, ptr %v3, i32 0, i32 2, !dbg !36
  %vec.current_size186 = load i64, ptr %vec.size_ptr184, align 4, !dbg !36
  %vec.current_cap187 = load i64, ptr %vec.cap_ptr185, align 4, !dbg !36
  %vec.data188 = load ptr, ptr %vec.data_ptr183, align 8, !dbg !36
  %vec.needs_alloc189 = icmp eq i64 %vec.current_cap187, 0, !dbg !36
  %vec.needs_grow190 = icmp eq i64 %vec.current_size186, %vec.current_cap187, !dbg !36
  %vec.needs_realloc191 = or i1 %vec.needs_alloc189, %vec.needs_grow190, !dbg !36
  br i1 %vec.needs_realloc191, label %vec.alloc192, label %vec.merge195, !dbg !36

vec.alloc192:                                     ; preds = %vec.merge172
  %12 = mul i64 %vec.current_cap187, 2, !dbg !36
  %vec.new_cap196 = select i1 %vec.needs_alloc189, i64 4, i64 %12, !dbg !36
  %vec.alloc_size197 = mul i64 %vec.new_cap196, 8, !dbg !36
  %vec.new_data198 = call ptr @malloc(i64 %vec.alloc_size197), !dbg !36
  %vec.has_data199 = icmp ne i64 %vec.current_size186, 0, !dbg !36
  br i1 %vec.has_data199, label %vec.copy193, label %vec.no_copy194, !dbg !36

vec.copy193:                                      ; preds = %vec.alloc192
  %vec.copy_size200 = mul i64 %vec.current_size186, 8, !dbg !36
  %13 = call ptr @memcpy(ptr %vec.new_data198, ptr %vec.data188, i64 %vec.copy_size200), !dbg !36
  br label %vec.no_copy194, !dbg !36

vec.no_copy194:                                   ; preds = %vec.copy193, %vec.alloc192
  store ptr %vec.new_data198, ptr %vec.data_ptr183, align 8, !dbg !36
  store i64 %vec.new_cap196, ptr %vec.cap_ptr185, align 4, !dbg !36
  br label %vec.merge195, !dbg !36

vec.merge195:                                     ; preds = %vec.no_copy194, %vec.merge172
  %vec.final_data201 = load ptr, ptr %vec.data_ptr183, align 8, !dbg !36
  %vec.reloaded_size202 = load i64, ptr %vec.size_ptr184, align 4, !dbg !36
  %vec.offset203 = mul i64 %vec.reloaded_size202, 8, !dbg !36
  %vec.element_ptr204 = getelementptr i8, ptr %vec.final_data201, i64 %vec.offset203, !dbg !36
  store i64 4, ptr %vec.element_ptr204, align 4, !dbg !36
  %vec.new_size205 = add i64 %vec.reloaded_size202, 1, !dbg !36
  store i64 %vec.new_size205, ptr %vec.size_ptr184, align 4, !dbg !36
  store i64 0, ptr %sum3, align 4, !dbg !36
  call void @llvm.dbg.declare(metadata ptr %sum3, metadata !24, metadata !DIExpression()), !dbg !44
  br label %for.init206, !dbg !36

for.init206:                                      ; preds = %vec.merge195
  store i1 true, ptr %__run_once_x211, align 1, !dbg !36
  call void @llvm.dbg.declare(metadata ptr %__run_once_x211, metadata !25, metadata !DIExpression()), !dbg !45
  br label %for.cond207, !dbg !36

for.cond207:                                      ; preds = %for.update209, %for.init206
  %__run_once_x212 = load i1, ptr %__run_once_x211, align 1, !dbg !36
  br i1 %__run_once_x212, label %for.body208, label %for.exit210, !dbg !36

for.body208:                                      ; preds = %for.cond207
  %vec.size_ptr213 = getelementptr inbounds { ptr, i64, i64 }, ptr %v3, i32 0, i32 1, !dbg !36
  %vec.len214 = load i64, ptr %vec.size_ptr213, align 4, !dbg !36
  store i64 %vec.len214, ptr %__len_x215, align 4, !dbg !36
  call void @llvm.dbg.declare(metadata ptr %__len_x215, metadata !26, metadata !DIExpression()), !dbg !45
  br label %for.init216, !dbg !36

for.update209:                                    ; preds = %for.exit220
  store i1 false, ptr %__run_once_x211, align 1, !dbg !36
  br label %for.cond207, !dbg !36

for.exit210:                                      ; preds = %for.cond207
  %sum3242 = load i64, ptr %sum3, align 4, !dbg !36
  %icmpneqtmp243 = icmp ne i64 %sum3242, 3, !dbg !36
  br i1 %icmpneqtmp243, label %then244, label %ifcont245, !dbg !36

for.init216:                                      ; preds = %for.body208
  store i64 0, ptr %__idx_x221, align 4, !dbg !36
  call void @llvm.dbg.declare(metadata ptr %__idx_x221, metadata !27, metadata !DIExpression()), !dbg !45
  br label %for.cond217, !dbg !36

for.cond217:                                      ; preds = %for.update219, %for.init216
  %__idx_x222 = load i64, ptr %__idx_x221, align 4, !dbg !36
  %__len_x223 = load i64, ptr %__len_x215, align 4, !dbg !36
  %icmpslttmp224 = icmp slt i64 %__idx_x222, %__len_x223, !dbg !36
  br i1 %icmpslttmp224, label %for.body218, label %for.exit220, !dbg !36

for.body218:                                      ; preds = %for.cond217
  %__idx_x225 = load i64, ptr %__idx_x221, align 4, !dbg !36
  %vec.data_ptr226 = getelementptr inbounds { ptr, i64, i64 }, ptr %v3, i32 0, i32 0, !dbg !36
  %vec.size_ptr227 = getelementptr inbounds { ptr, i64, i64 }, ptr %v3, i32 0, i32 1, !dbg !36
  %vec.data228 = load ptr, ptr %vec.data_ptr226, align 8, !dbg !36
  %vec.size229 = load i64, ptr %vec.size_ptr227, align 4, !dbg !36
  %vec.offset230 = mul i64 %__idx_x225, 8, !dbg !36
  %vec.element_ptr231 = getelementptr i8, ptr %vec.data228, i64 %vec.offset230, !dbg !36
  %vec.element232 = load i64, ptr %vec.element_ptr231, align 4, !dbg !36
  store i64 %vec.element232, ptr %x233, align 4, !dbg !36
  call void @llvm.dbg.declare(metadata ptr %x233, metadata !28, metadata !DIExpression()), !dbg !45
  %sum3234 = load i64, ptr %sum3, align 4, !dbg !36
  %icmpsgetmp = icmp sge i64 %sum3234, 3, !dbg !36
  br i1 %icmpsgetmp, label %then235, label %ifcont236, !dbg !36

for.update219:                                    ; preds = %ifcont236
  %__idx_x240 = load i64, ptr %__idx_x221, align 4, !dbg !36
  %addtmp241 = add i64 %__idx_x240, 1, !dbg !36
  store i64 %addtmp241, ptr %__idx_x221, align 4, !dbg !36
  br label %for.cond217, !dbg !36

for.exit220:                                      ; preds = %then235, %for.cond217
  br label %for.update209, !dbg !36

then235:                                          ; preds = %for.body218
  br label %for.exit220, !dbg !36

ifcont236:                                        ; preds = %for.body218
  %sum3237 = load i64, ptr %sum3, align 4, !dbg !36
  %x238 = load i64, ptr %x233, align 4, !dbg !36
  %addtmp239 = add i64 %sum3237, %x238, !dbg !36
  store i64 %addtmp239, ptr %sum3, align 4, !dbg !36
  br label %for.update219, !dbg !36

then244:                                          ; preds = %for.exit210
  ret i64 3, !dbg !36

ifcont245:                                        ; preds = %for.exit210
  %vec.new246 = alloca { ptr, i64, i64 }, align 8, !dbg !36
  %vec.ptr_field247 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new246, i32 0, i32 0, !dbg !36
  store ptr null, ptr %vec.ptr_field247, align 8, !dbg !36
  %vec.size_field248 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new246, i32 0, i32 1, !dbg !36
  store i64 0, ptr %vec.size_field248, align 4, !dbg !36
  %vec.cap_field249 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new246, i32 0, i32 2, !dbg !36
  store i64 0, ptr %vec.cap_field249, align 4, !dbg !36
  %vec.new.value250 = load { ptr, i64, i64 }, ptr %vec.new246, align 8, !dbg !36
  store { ptr, i64, i64 } %vec.new.value250, ptr %v4, align 8, !dbg !36
  call void @llvm.dbg.declare(metadata ptr %v4, metadata !29, metadata !DIExpression()), !dbg !46
  %vec.data_ptr251 = getelementptr inbounds { ptr, i64, i64 }, ptr %v4, i32 0, i32 0, !dbg !36
  %vec.size_ptr252 = getelementptr inbounds { ptr, i64, i64 }, ptr %v4, i32 0, i32 1, !dbg !36
  %vec.cap_ptr253 = getelementptr inbounds { ptr, i64, i64 }, ptr %v4, i32 0, i32 2, !dbg !36
  %vec.current_size254 = load i64, ptr %vec.size_ptr252, align 4, !dbg !36
  %vec.current_cap255 = load i64, ptr %vec.cap_ptr253, align 4, !dbg !36
  %vec.data256 = load ptr, ptr %vec.data_ptr251, align 8, !dbg !36
  %vec.needs_alloc257 = icmp eq i64 %vec.current_cap255, 0, !dbg !36
  %vec.needs_grow258 = icmp eq i64 %vec.current_size254, %vec.current_cap255, !dbg !36
  %vec.needs_realloc259 = or i1 %vec.needs_alloc257, %vec.needs_grow258, !dbg !36
  br i1 %vec.needs_realloc259, label %vec.alloc260, label %vec.merge263, !dbg !36

vec.alloc260:                                     ; preds = %ifcont245
  %14 = mul i64 %vec.current_cap255, 2, !dbg !36
  %vec.new_cap264 = select i1 %vec.needs_alloc257, i64 4, i64 %14, !dbg !36
  %vec.alloc_size265 = mul i64 %vec.new_cap264, 8, !dbg !36
  %vec.new_data266 = call ptr @malloc(i64 %vec.alloc_size265), !dbg !36
  %vec.has_data267 = icmp ne i64 %vec.current_size254, 0, !dbg !36
  br i1 %vec.has_data267, label %vec.copy261, label %vec.no_copy262, !dbg !36

vec.copy261:                                      ; preds = %vec.alloc260
  %vec.copy_size268 = mul i64 %vec.current_size254, 8, !dbg !36
  %15 = call ptr @memcpy(ptr %vec.new_data266, ptr %vec.data256, i64 %vec.copy_size268), !dbg !36
  br label %vec.no_copy262, !dbg !36

vec.no_copy262:                                   ; preds = %vec.copy261, %vec.alloc260
  store ptr %vec.new_data266, ptr %vec.data_ptr251, align 8, !dbg !36
  store i64 %vec.new_cap264, ptr %vec.cap_ptr253, align 4, !dbg !36
  br label %vec.merge263, !dbg !36

vec.merge263:                                     ; preds = %vec.no_copy262, %ifcont245
  %vec.final_data269 = load ptr, ptr %vec.data_ptr251, align 8, !dbg !36
  %vec.reloaded_size270 = load i64, ptr %vec.size_ptr252, align 4, !dbg !36
  %vec.offset271 = mul i64 %vec.reloaded_size270, 8, !dbg !36
  %vec.element_ptr272 = getelementptr i8, ptr %vec.final_data269, i64 %vec.offset271, !dbg !36
  store i64 1, ptr %vec.element_ptr272, align 4, !dbg !36
  %vec.new_size273 = add i64 %vec.reloaded_size270, 1, !dbg !36
  store i64 %vec.new_size273, ptr %vec.size_ptr252, align 4, !dbg !36
  %vec.data_ptr274 = getelementptr inbounds { ptr, i64, i64 }, ptr %v4, i32 0, i32 0, !dbg !36
  %vec.size_ptr275 = getelementptr inbounds { ptr, i64, i64 }, ptr %v4, i32 0, i32 1, !dbg !36
  %vec.cap_ptr276 = getelementptr inbounds { ptr, i64, i64 }, ptr %v4, i32 0, i32 2, !dbg !36
  %vec.current_size277 = load i64, ptr %vec.size_ptr275, align 4, !dbg !36
  %vec.current_cap278 = load i64, ptr %vec.cap_ptr276, align 4, !dbg !36
  %vec.data279 = load ptr, ptr %vec.data_ptr274, align 8, !dbg !36
  %vec.needs_alloc280 = icmp eq i64 %vec.current_cap278, 0, !dbg !36
  %vec.needs_grow281 = icmp eq i64 %vec.current_size277, %vec.current_cap278, !dbg !36
  %vec.needs_realloc282 = or i1 %vec.needs_alloc280, %vec.needs_grow281, !dbg !36
  br i1 %vec.needs_realloc282, label %vec.alloc283, label %vec.merge286, !dbg !36

vec.alloc283:                                     ; preds = %vec.merge263
  %16 = mul i64 %vec.current_cap278, 2, !dbg !36
  %vec.new_cap287 = select i1 %vec.needs_alloc280, i64 4, i64 %16, !dbg !36
  %vec.alloc_size288 = mul i64 %vec.new_cap287, 8, !dbg !36
  %vec.new_data289 = call ptr @malloc(i64 %vec.alloc_size288), !dbg !36
  %vec.has_data290 = icmp ne i64 %vec.current_size277, 0, !dbg !36
  br i1 %vec.has_data290, label %vec.copy284, label %vec.no_copy285, !dbg !36

vec.copy284:                                      ; preds = %vec.alloc283
  %vec.copy_size291 = mul i64 %vec.current_size277, 8, !dbg !36
  %17 = call ptr @memcpy(ptr %vec.new_data289, ptr %vec.data279, i64 %vec.copy_size291), !dbg !36
  br label %vec.no_copy285, !dbg !36

vec.no_copy285:                                   ; preds = %vec.copy284, %vec.alloc283
  store ptr %vec.new_data289, ptr %vec.data_ptr274, align 8, !dbg !36
  store i64 %vec.new_cap287, ptr %vec.cap_ptr276, align 4, !dbg !36
  br label %vec.merge286, !dbg !36

vec.merge286:                                     ; preds = %vec.no_copy285, %vec.merge263
  %vec.final_data292 = load ptr, ptr %vec.data_ptr274, align 8, !dbg !36
  %vec.reloaded_size293 = load i64, ptr %vec.size_ptr275, align 4, !dbg !36
  %vec.offset294 = mul i64 %vec.reloaded_size293, 8, !dbg !36
  %vec.element_ptr295 = getelementptr i8, ptr %vec.final_data292, i64 %vec.offset294, !dbg !36
  store i64 2, ptr %vec.element_ptr295, align 4, !dbg !36
  %vec.new_size296 = add i64 %vec.reloaded_size293, 1, !dbg !36
  store i64 %vec.new_size296, ptr %vec.size_ptr275, align 4, !dbg !36
  %vec.data_ptr297 = getelementptr inbounds { ptr, i64, i64 }, ptr %v4, i32 0, i32 0, !dbg !36
  %vec.size_ptr298 = getelementptr inbounds { ptr, i64, i64 }, ptr %v4, i32 0, i32 1, !dbg !36
  %vec.cap_ptr299 = getelementptr inbounds { ptr, i64, i64 }, ptr %v4, i32 0, i32 2, !dbg !36
  %vec.current_size300 = load i64, ptr %vec.size_ptr298, align 4, !dbg !36
  %vec.current_cap301 = load i64, ptr %vec.cap_ptr299, align 4, !dbg !36
  %vec.data302 = load ptr, ptr %vec.data_ptr297, align 8, !dbg !36
  %vec.needs_alloc303 = icmp eq i64 %vec.current_cap301, 0, !dbg !36
  %vec.needs_grow304 = icmp eq i64 %vec.current_size300, %vec.current_cap301, !dbg !36
  %vec.needs_realloc305 = or i1 %vec.needs_alloc303, %vec.needs_grow304, !dbg !36
  br i1 %vec.needs_realloc305, label %vec.alloc306, label %vec.merge309, !dbg !36

vec.alloc306:                                     ; preds = %vec.merge286
  %18 = mul i64 %vec.current_cap301, 2, !dbg !36
  %vec.new_cap310 = select i1 %vec.needs_alloc303, i64 4, i64 %18, !dbg !36
  %vec.alloc_size311 = mul i64 %vec.new_cap310, 8, !dbg !36
  %vec.new_data312 = call ptr @malloc(i64 %vec.alloc_size311), !dbg !36
  %vec.has_data313 = icmp ne i64 %vec.current_size300, 0, !dbg !36
  br i1 %vec.has_data313, label %vec.copy307, label %vec.no_copy308, !dbg !36

vec.copy307:                                      ; preds = %vec.alloc306
  %vec.copy_size314 = mul i64 %vec.current_size300, 8, !dbg !36
  %19 = call ptr @memcpy(ptr %vec.new_data312, ptr %vec.data302, i64 %vec.copy_size314), !dbg !36
  br label %vec.no_copy308, !dbg !36

vec.no_copy308:                                   ; preds = %vec.copy307, %vec.alloc306
  store ptr %vec.new_data312, ptr %vec.data_ptr297, align 8, !dbg !36
  store i64 %vec.new_cap310, ptr %vec.cap_ptr299, align 4, !dbg !36
  br label %vec.merge309, !dbg !36

vec.merge309:                                     ; preds = %vec.no_copy308, %vec.merge286
  %vec.final_data315 = load ptr, ptr %vec.data_ptr297, align 8, !dbg !36
  %vec.reloaded_size316 = load i64, ptr %vec.size_ptr298, align 4, !dbg !36
  %vec.offset317 = mul i64 %vec.reloaded_size316, 8, !dbg !36
  %vec.element_ptr318 = getelementptr i8, ptr %vec.final_data315, i64 %vec.offset317, !dbg !36
  store i64 3, ptr %vec.element_ptr318, align 4, !dbg !36
  %vec.new_size319 = add i64 %vec.reloaded_size316, 1, !dbg !36
  store i64 %vec.new_size319, ptr %vec.size_ptr298, align 4, !dbg !36
  %vec.data_ptr320 = getelementptr inbounds { ptr, i64, i64 }, ptr %v4, i32 0, i32 0, !dbg !36
  %vec.size_ptr321 = getelementptr inbounds { ptr, i64, i64 }, ptr %v4, i32 0, i32 1, !dbg !36
  %vec.cap_ptr322 = getelementptr inbounds { ptr, i64, i64 }, ptr %v4, i32 0, i32 2, !dbg !36
  %vec.current_size323 = load i64, ptr %vec.size_ptr321, align 4, !dbg !36
  %vec.current_cap324 = load i64, ptr %vec.cap_ptr322, align 4, !dbg !36
  %vec.data325 = load ptr, ptr %vec.data_ptr320, align 8, !dbg !36
  %vec.needs_alloc326 = icmp eq i64 %vec.current_cap324, 0, !dbg !36
  %vec.needs_grow327 = icmp eq i64 %vec.current_size323, %vec.current_cap324, !dbg !36
  %vec.needs_realloc328 = or i1 %vec.needs_alloc326, %vec.needs_grow327, !dbg !36
  br i1 %vec.needs_realloc328, label %vec.alloc329, label %vec.merge332, !dbg !36

vec.alloc329:                                     ; preds = %vec.merge309
  %20 = mul i64 %vec.current_cap324, 2, !dbg !36
  %vec.new_cap333 = select i1 %vec.needs_alloc326, i64 4, i64 %20, !dbg !36
  %vec.alloc_size334 = mul i64 %vec.new_cap333, 8, !dbg !36
  %vec.new_data335 = call ptr @malloc(i64 %vec.alloc_size334), !dbg !36
  %vec.has_data336 = icmp ne i64 %vec.current_size323, 0, !dbg !36
  br i1 %vec.has_data336, label %vec.copy330, label %vec.no_copy331, !dbg !36

vec.copy330:                                      ; preds = %vec.alloc329
  %vec.copy_size337 = mul i64 %vec.current_size323, 8, !dbg !36
  %21 = call ptr @memcpy(ptr %vec.new_data335, ptr %vec.data325, i64 %vec.copy_size337), !dbg !36
  br label %vec.no_copy331, !dbg !36

vec.no_copy331:                                   ; preds = %vec.copy330, %vec.alloc329
  store ptr %vec.new_data335, ptr %vec.data_ptr320, align 8, !dbg !36
  store i64 %vec.new_cap333, ptr %vec.cap_ptr322, align 4, !dbg !36
  br label %vec.merge332, !dbg !36

vec.merge332:                                     ; preds = %vec.no_copy331, %vec.merge309
  %vec.final_data338 = load ptr, ptr %vec.data_ptr320, align 8, !dbg !36
  %vec.reloaded_size339 = load i64, ptr %vec.size_ptr321, align 4, !dbg !36
  %vec.offset340 = mul i64 %vec.reloaded_size339, 8, !dbg !36
  %vec.element_ptr341 = getelementptr i8, ptr %vec.final_data338, i64 %vec.offset340, !dbg !36
  store i64 4, ptr %vec.element_ptr341, align 4, !dbg !36
  %vec.new_size342 = add i64 %vec.reloaded_size339, 1, !dbg !36
  store i64 %vec.new_size342, ptr %vec.size_ptr321, align 4, !dbg !36
  store i64 0, ptr %sum4, align 4, !dbg !36
  call void @llvm.dbg.declare(metadata ptr %sum4, metadata !30, metadata !DIExpression()), !dbg !47
  store i64 0, ptr %skip_pos, align 4, !dbg !36
  call void @llvm.dbg.declare(metadata ptr %skip_pos, metadata !31, metadata !DIExpression()), !dbg !48
  br label %for.init343, !dbg !36

for.init343:                                      ; preds = %vec.merge332
  store i1 true, ptr %__run_once_x348, align 1, !dbg !36
  call void @llvm.dbg.declare(metadata ptr %__run_once_x348, metadata !32, metadata !DIExpression()), !dbg !49
  br label %for.cond344, !dbg !36

for.cond344:                                      ; preds = %for.update346, %for.init343
  %__run_once_x349 = load i1, ptr %__run_once_x348, align 1, !dbg !36
  br i1 %__run_once_x349, label %for.body345, label %for.exit347, !dbg !36

for.body345:                                      ; preds = %for.cond344
  %vec.size_ptr350 = getelementptr inbounds { ptr, i64, i64 }, ptr %v4, i32 0, i32 1, !dbg !36
  %vec.len351 = load i64, ptr %vec.size_ptr350, align 4, !dbg !36
  store i64 %vec.len351, ptr %__len_x352, align 4, !dbg !36
  call void @llvm.dbg.declare(metadata ptr %__len_x352, metadata !33, metadata !DIExpression()), !dbg !49
  br label %for.init353, !dbg !36

for.update346:                                    ; preds = %for.exit357
  store i1 false, ptr %__run_once_x348, align 1, !dbg !36
  br label %for.cond344, !dbg !36

for.exit347:                                      ; preds = %for.cond344
  %sum4381 = load i64, ptr %sum4, align 4, !dbg !36
  %icmpneqtmp382 = icmp ne i64 %sum4381, 8, !dbg !36
  br i1 %icmpneqtmp382, label %then383, label %ifcont384, !dbg !36

for.init353:                                      ; preds = %for.body345
  store i64 0, ptr %__idx_x358, align 4, !dbg !36
  call void @llvm.dbg.declare(metadata ptr %__idx_x358, metadata !34, metadata !DIExpression()), !dbg !49
  br label %for.cond354, !dbg !36

for.cond354:                                      ; preds = %for.update356, %for.init353
  %__idx_x359 = load i64, ptr %__idx_x358, align 4, !dbg !36
  %__len_x360 = load i64, ptr %__len_x352, align 4, !dbg !36
  %icmpslttmp361 = icmp slt i64 %__idx_x359, %__len_x360, !dbg !36
  br i1 %icmpslttmp361, label %for.body355, label %for.exit357, !dbg !36

for.body355:                                      ; preds = %for.cond354
  %__idx_x362 = load i64, ptr %__idx_x358, align 4, !dbg !36
  %vec.data_ptr363 = getelementptr inbounds { ptr, i64, i64 }, ptr %v4, i32 0, i32 0, !dbg !36
  %vec.size_ptr364 = getelementptr inbounds { ptr, i64, i64 }, ptr %v4, i32 0, i32 1, !dbg !36
  %vec.data365 = load ptr, ptr %vec.data_ptr363, align 8, !dbg !36
  %vec.size366 = load i64, ptr %vec.size_ptr364, align 4, !dbg !36
  %vec.offset367 = mul i64 %__idx_x362, 8, !dbg !36
  %vec.element_ptr368 = getelementptr i8, ptr %vec.data365, i64 %vec.offset367, !dbg !36
  %vec.element369 = load i64, ptr %vec.element_ptr368, align 4, !dbg !36
  store i64 %vec.element369, ptr %x370, align 4, !dbg !36
  call void @llvm.dbg.declare(metadata ptr %x370, metadata !35, metadata !DIExpression()), !dbg !49
  %skip_pos371 = load i64, ptr %skip_pos, align 4, !dbg !36
  %addtmp372 = add i64 %skip_pos371, 1, !dbg !36
  store i64 %addtmp372, ptr %skip_pos, align 4, !dbg !36
  %skip_pos373 = load i64, ptr %skip_pos, align 4, !dbg !36
  %icmpeqtmp = icmp eq i64 %skip_pos373, 2, !dbg !36
  br i1 %icmpeqtmp, label %then374, label %ifcont375, !dbg !36

for.update356:                                    ; preds = %ifcont375, %then374
  %__idx_x379 = load i64, ptr %__idx_x358, align 4, !dbg !36
  %addtmp380 = add i64 %__idx_x379, 1, !dbg !36
  store i64 %addtmp380, ptr %__idx_x358, align 4, !dbg !36
  br label %for.cond354, !dbg !36

for.exit357:                                      ; preds = %for.cond354
  br label %for.update346, !dbg !36

then374:                                          ; preds = %for.body355
  br label %for.update356, !dbg !36

ifcont375:                                        ; preds = %for.body355
  %sum4376 = load i64, ptr %sum4, align 4, !dbg !36
  %x377 = load i64, ptr %x370, align 4, !dbg !36
  %addtmp378 = add i64 %sum4376, %x377, !dbg !36
  store i64 %addtmp378, ptr %sum4, align 4, !dbg !36
  br label %for.update356, !dbg !36

then383:                                          ; preds = %for.exit347
  ret i64 4, !dbg !36

ifcont384:                                        ; preds = %for.exit347
  ret i64 0, !dbg !36
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @malloc(i64)

declare ptr @memcpy(ptr, ptr, i64)

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "comprehensive_test.vyn.ll", directory: "/home/rick/Projects/Vyn/test/vec_for")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11, !12, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35}
!9 = !DILocalVariable(name: "v1", scope: !4, file: !1, line: 4, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_{ ptr, i64, i64 }", scope: !1, file: !1, size: 192, align: 8)
!11 = !DILocalVariable(name: "sum1", scope: !4, file: !1, line: 9, type: !7)
!12 = !DILocalVariable(name: "__run_once_x", scope: !4, file: !1, line: 10, type: !13)
!13 = !DIBasicType(name: "bool", size: 1, encoding: DW_ATE_boolean)
!14 = !DILocalVariable(name: "__len_x", scope: !4, file: !1, line: 10, type: !7)
!15 = !DILocalVariable(name: "__idx_x", scope: !4, file: !1, line: 10, type: !7)
!16 = !DILocalVariable(name: "x", scope: !4, file: !1, line: 10, type: !7)
!17 = !DILocalVariable(name: "v2", scope: !4, file: !1, line: 17, type: !10)
!18 = !DILocalVariable(name: "sum2", scope: !4, file: !1, line: 19, type: !7)
!19 = !DILocalVariable(name: "__run_once_x", scope: !4, file: !1, line: 20, type: !13)
!20 = !DILocalVariable(name: "__len_x", scope: !4, file: !1, line: 20, type: !7)
!21 = !DILocalVariable(name: "__idx_x", scope: !4, file: !1, line: 20, type: !7)
!22 = !DILocalVariable(name: "x", scope: !4, file: !1, line: 20, type: !7)
!23 = !DILocalVariable(name: "v3", scope: !4, file: !1, line: 27, type: !10)
!24 = !DILocalVariable(name: "sum3", scope: !4, file: !1, line: 33, type: !7)
!25 = !DILocalVariable(name: "__run_once_x", scope: !4, file: !1, line: 34, type: !13)
!26 = !DILocalVariable(name: "__len_x", scope: !4, file: !1, line: 34, type: !7)
!27 = !DILocalVariable(name: "__idx_x", scope: !4, file: !1, line: 34, type: !7)
!28 = !DILocalVariable(name: "x", scope: !4, file: !1, line: 34, type: !7)
!29 = !DILocalVariable(name: "v4", scope: !4, file: !1, line: 44, type: !10)
!30 = !DILocalVariable(name: "sum4", scope: !4, file: !1, line: 50, type: !7)
!31 = !DILocalVariable(name: "skip_pos", scope: !4, file: !1, line: 51, type: !7)
!32 = !DILocalVariable(name: "__run_once_x", scope: !4, file: !1, line: 52, type: !13)
!33 = !DILocalVariable(name: "__len_x", scope: !4, file: !1, line: 52, type: !7)
!34 = !DILocalVariable(name: "__idx_x", scope: !4, file: !1, line: 52, type: !7)
!35 = !DILocalVariable(name: "x", scope: !4, file: !1, line: 52, type: !7)
!36 = !DILocation(line: 3, column: 1, scope: !4)
!37 = !DILocation(line: 4, column: 5, scope: !4)
!38 = !DILocation(line: 9, column: 1, scope: !4)
!39 = !DILocation(line: 10, column: 10, scope: !4)
!40 = !DILocation(line: 17, column: 5, scope: !4)
!41 = !DILocation(line: 19, column: 1, scope: !4)
!42 = !DILocation(line: 20, column: 10, scope: !4)
!43 = !DILocation(line: 27, column: 5, scope: !4)
!44 = !DILocation(line: 33, column: 1, scope: !4)
!45 = !DILocation(line: 34, column: 10, scope: !4)
!46 = !DILocation(line: 44, column: 5, scope: !4)
!47 = !DILocation(line: 50, column: 1, scope: !4)
!48 = !DILocation(line: 51, column: 1, scope: !4)
!49 = !DILocation(line: 52, column: 10, scope: !4)
