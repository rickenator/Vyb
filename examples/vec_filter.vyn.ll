; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() !dbg !4 {
entry:
  %even = alloca i64, align 8, !dbg !24
  %__idx_even = alloca i64, align 8, !dbg !24
  %__len_even = alloca i64, align 8, !dbg !24
  %__run_once_even = alloca i1, align 1, !dbg !24
  %sum = alloca i64, align 8, !dbg !24
  %doubled = alloca i64, align 8, !dbg !24
  %half = alloca i64, align 8, !dbg !24
  %num = alloca i64, align 8, !dbg !24
  %__idx_num = alloca i64, align 8, !dbg !24
  %__len_num = alloca i64, align 8, !dbg !24
  %__run_once_num = alloca i1, align 1, !dbg !24
  %evens = alloca { ptr, i64, i64 }, align 8, !dbg !24
  %numbers = alloca { ptr, i64, i64 }, align 8, !dbg !24
  %vec.new = alloca { ptr, i64, i64 }, align 8, !dbg !24
  %vec.ptr_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 0, !dbg !24
  store ptr null, ptr %vec.ptr_field, align 8, !dbg !24
  %vec.size_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 1, !dbg !24
  store i64 0, ptr %vec.size_field, align 4, !dbg !24
  %vec.cap_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 2, !dbg !24
  store i64 0, ptr %vec.cap_field, align 4, !dbg !24
  %vec.new.value = load { ptr, i64, i64 }, ptr %vec.new, align 8, !dbg !24
  store { ptr, i64, i64 } %vec.new.value, ptr %numbers, align 8, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %numbers, metadata !9, metadata !DIExpression()), !dbg !25
  %vec.data_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 0, !dbg !24
  %vec.size_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !24
  %vec.cap_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 2, !dbg !24
  %vec.current_size = load i64, ptr %vec.size_ptr, align 4, !dbg !24
  %vec.current_cap = load i64, ptr %vec.cap_ptr, align 4, !dbg !24
  %vec.data = load ptr, ptr %vec.data_ptr, align 8, !dbg !24
  %vec.needs_alloc = icmp eq i64 %vec.current_cap, 0, !dbg !24
  %vec.needs_grow = icmp eq i64 %vec.current_size, %vec.current_cap, !dbg !24
  %vec.needs_realloc = or i1 %vec.needs_alloc, %vec.needs_grow, !dbg !24
  br i1 %vec.needs_realloc, label %vec.alloc, label %vec.merge, !dbg !24

vec.alloc:                                        ; preds = %entry
  %0 = mul i64 %vec.current_cap, 2, !dbg !24
  %vec.new_cap = select i1 %vec.needs_alloc, i64 4, i64 %0, !dbg !24
  %vec.alloc_size = mul i64 %vec.new_cap, 8, !dbg !24
  %vec.new_data = call ptr @malloc(i64 %vec.alloc_size), !dbg !24
  %vec.has_data = icmp ne i64 %vec.current_size, 0, !dbg !24
  br i1 %vec.has_data, label %vec.copy, label %vec.no_copy, !dbg !24

vec.copy:                                         ; preds = %vec.alloc
  %vec.copy_size = mul i64 %vec.current_size, 8, !dbg !24
  %1 = call ptr @memcpy(ptr %vec.new_data, ptr %vec.data, i64 %vec.copy_size), !dbg !24
  br label %vec.no_copy, !dbg !24

vec.no_copy:                                      ; preds = %vec.copy, %vec.alloc
  store ptr %vec.new_data, ptr %vec.data_ptr, align 8, !dbg !24
  store i64 %vec.new_cap, ptr %vec.cap_ptr, align 4, !dbg !24
  br label %vec.merge, !dbg !24

vec.merge:                                        ; preds = %vec.no_copy, %entry
  %vec.final_data = load ptr, ptr %vec.data_ptr, align 8, !dbg !24
  %vec.reloaded_size = load i64, ptr %vec.size_ptr, align 4, !dbg !24
  %vec.offset = mul i64 %vec.reloaded_size, 8, !dbg !24
  %vec.element_ptr = getelementptr i8, ptr %vec.final_data, i64 %vec.offset, !dbg !24
  store i64 1, ptr %vec.element_ptr, align 4, !dbg !24
  %vec.new_size = add i64 %vec.reloaded_size, 1, !dbg !24
  store i64 %vec.new_size, ptr %vec.size_ptr, align 4, !dbg !24
  %vec.data_ptr1 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 0, !dbg !24
  %vec.size_ptr2 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !24
  %vec.cap_ptr3 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 2, !dbg !24
  %vec.current_size4 = load i64, ptr %vec.size_ptr2, align 4, !dbg !24
  %vec.current_cap5 = load i64, ptr %vec.cap_ptr3, align 4, !dbg !24
  %vec.data6 = load ptr, ptr %vec.data_ptr1, align 8, !dbg !24
  %vec.needs_alloc7 = icmp eq i64 %vec.current_cap5, 0, !dbg !24
  %vec.needs_grow8 = icmp eq i64 %vec.current_size4, %vec.current_cap5, !dbg !24
  %vec.needs_realloc9 = or i1 %vec.needs_alloc7, %vec.needs_grow8, !dbg !24
  br i1 %vec.needs_realloc9, label %vec.alloc10, label %vec.merge13, !dbg !24

vec.alloc10:                                      ; preds = %vec.merge
  %2 = mul i64 %vec.current_cap5, 2, !dbg !24
  %vec.new_cap14 = select i1 %vec.needs_alloc7, i64 4, i64 %2, !dbg !24
  %vec.alloc_size15 = mul i64 %vec.new_cap14, 8, !dbg !24
  %vec.new_data16 = call ptr @malloc(i64 %vec.alloc_size15), !dbg !24
  %vec.has_data17 = icmp ne i64 %vec.current_size4, 0, !dbg !24
  br i1 %vec.has_data17, label %vec.copy11, label %vec.no_copy12, !dbg !24

vec.copy11:                                       ; preds = %vec.alloc10
  %vec.copy_size18 = mul i64 %vec.current_size4, 8, !dbg !24
  %3 = call ptr @memcpy(ptr %vec.new_data16, ptr %vec.data6, i64 %vec.copy_size18), !dbg !24
  br label %vec.no_copy12, !dbg !24

vec.no_copy12:                                    ; preds = %vec.copy11, %vec.alloc10
  store ptr %vec.new_data16, ptr %vec.data_ptr1, align 8, !dbg !24
  store i64 %vec.new_cap14, ptr %vec.cap_ptr3, align 4, !dbg !24
  br label %vec.merge13, !dbg !24

vec.merge13:                                      ; preds = %vec.no_copy12, %vec.merge
  %vec.final_data19 = load ptr, ptr %vec.data_ptr1, align 8, !dbg !24
  %vec.reloaded_size20 = load i64, ptr %vec.size_ptr2, align 4, !dbg !24
  %vec.offset21 = mul i64 %vec.reloaded_size20, 8, !dbg !24
  %vec.element_ptr22 = getelementptr i8, ptr %vec.final_data19, i64 %vec.offset21, !dbg !24
  store i64 2, ptr %vec.element_ptr22, align 4, !dbg !24
  %vec.new_size23 = add i64 %vec.reloaded_size20, 1, !dbg !24
  store i64 %vec.new_size23, ptr %vec.size_ptr2, align 4, !dbg !24
  %vec.data_ptr24 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 0, !dbg !24
  %vec.size_ptr25 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !24
  %vec.cap_ptr26 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 2, !dbg !24
  %vec.current_size27 = load i64, ptr %vec.size_ptr25, align 4, !dbg !24
  %vec.current_cap28 = load i64, ptr %vec.cap_ptr26, align 4, !dbg !24
  %vec.data29 = load ptr, ptr %vec.data_ptr24, align 8, !dbg !24
  %vec.needs_alloc30 = icmp eq i64 %vec.current_cap28, 0, !dbg !24
  %vec.needs_grow31 = icmp eq i64 %vec.current_size27, %vec.current_cap28, !dbg !24
  %vec.needs_realloc32 = or i1 %vec.needs_alloc30, %vec.needs_grow31, !dbg !24
  br i1 %vec.needs_realloc32, label %vec.alloc33, label %vec.merge36, !dbg !24

vec.alloc33:                                      ; preds = %vec.merge13
  %4 = mul i64 %vec.current_cap28, 2, !dbg !24
  %vec.new_cap37 = select i1 %vec.needs_alloc30, i64 4, i64 %4, !dbg !24
  %vec.alloc_size38 = mul i64 %vec.new_cap37, 8, !dbg !24
  %vec.new_data39 = call ptr @malloc(i64 %vec.alloc_size38), !dbg !24
  %vec.has_data40 = icmp ne i64 %vec.current_size27, 0, !dbg !24
  br i1 %vec.has_data40, label %vec.copy34, label %vec.no_copy35, !dbg !24

vec.copy34:                                       ; preds = %vec.alloc33
  %vec.copy_size41 = mul i64 %vec.current_size27, 8, !dbg !24
  %5 = call ptr @memcpy(ptr %vec.new_data39, ptr %vec.data29, i64 %vec.copy_size41), !dbg !24
  br label %vec.no_copy35, !dbg !24

vec.no_copy35:                                    ; preds = %vec.copy34, %vec.alloc33
  store ptr %vec.new_data39, ptr %vec.data_ptr24, align 8, !dbg !24
  store i64 %vec.new_cap37, ptr %vec.cap_ptr26, align 4, !dbg !24
  br label %vec.merge36, !dbg !24

vec.merge36:                                      ; preds = %vec.no_copy35, %vec.merge13
  %vec.final_data42 = load ptr, ptr %vec.data_ptr24, align 8, !dbg !24
  %vec.reloaded_size43 = load i64, ptr %vec.size_ptr25, align 4, !dbg !24
  %vec.offset44 = mul i64 %vec.reloaded_size43, 8, !dbg !24
  %vec.element_ptr45 = getelementptr i8, ptr %vec.final_data42, i64 %vec.offset44, !dbg !24
  store i64 3, ptr %vec.element_ptr45, align 4, !dbg !24
  %vec.new_size46 = add i64 %vec.reloaded_size43, 1, !dbg !24
  store i64 %vec.new_size46, ptr %vec.size_ptr25, align 4, !dbg !24
  %vec.data_ptr47 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 0, !dbg !24
  %vec.size_ptr48 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !24
  %vec.cap_ptr49 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 2, !dbg !24
  %vec.current_size50 = load i64, ptr %vec.size_ptr48, align 4, !dbg !24
  %vec.current_cap51 = load i64, ptr %vec.cap_ptr49, align 4, !dbg !24
  %vec.data52 = load ptr, ptr %vec.data_ptr47, align 8, !dbg !24
  %vec.needs_alloc53 = icmp eq i64 %vec.current_cap51, 0, !dbg !24
  %vec.needs_grow54 = icmp eq i64 %vec.current_size50, %vec.current_cap51, !dbg !24
  %vec.needs_realloc55 = or i1 %vec.needs_alloc53, %vec.needs_grow54, !dbg !24
  br i1 %vec.needs_realloc55, label %vec.alloc56, label %vec.merge59, !dbg !24

vec.alloc56:                                      ; preds = %vec.merge36
  %6 = mul i64 %vec.current_cap51, 2, !dbg !24
  %vec.new_cap60 = select i1 %vec.needs_alloc53, i64 4, i64 %6, !dbg !24
  %vec.alloc_size61 = mul i64 %vec.new_cap60, 8, !dbg !24
  %vec.new_data62 = call ptr @malloc(i64 %vec.alloc_size61), !dbg !24
  %vec.has_data63 = icmp ne i64 %vec.current_size50, 0, !dbg !24
  br i1 %vec.has_data63, label %vec.copy57, label %vec.no_copy58, !dbg !24

vec.copy57:                                       ; preds = %vec.alloc56
  %vec.copy_size64 = mul i64 %vec.current_size50, 8, !dbg !24
  %7 = call ptr @memcpy(ptr %vec.new_data62, ptr %vec.data52, i64 %vec.copy_size64), !dbg !24
  br label %vec.no_copy58, !dbg !24

vec.no_copy58:                                    ; preds = %vec.copy57, %vec.alloc56
  store ptr %vec.new_data62, ptr %vec.data_ptr47, align 8, !dbg !24
  store i64 %vec.new_cap60, ptr %vec.cap_ptr49, align 4, !dbg !24
  br label %vec.merge59, !dbg !24

vec.merge59:                                      ; preds = %vec.no_copy58, %vec.merge36
  %vec.final_data65 = load ptr, ptr %vec.data_ptr47, align 8, !dbg !24
  %vec.reloaded_size66 = load i64, ptr %vec.size_ptr48, align 4, !dbg !24
  %vec.offset67 = mul i64 %vec.reloaded_size66, 8, !dbg !24
  %vec.element_ptr68 = getelementptr i8, ptr %vec.final_data65, i64 %vec.offset67, !dbg !24
  store i64 4, ptr %vec.element_ptr68, align 4, !dbg !24
  %vec.new_size69 = add i64 %vec.reloaded_size66, 1, !dbg !24
  store i64 %vec.new_size69, ptr %vec.size_ptr48, align 4, !dbg !24
  %vec.data_ptr70 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 0, !dbg !24
  %vec.size_ptr71 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !24
  %vec.cap_ptr72 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 2, !dbg !24
  %vec.current_size73 = load i64, ptr %vec.size_ptr71, align 4, !dbg !24
  %vec.current_cap74 = load i64, ptr %vec.cap_ptr72, align 4, !dbg !24
  %vec.data75 = load ptr, ptr %vec.data_ptr70, align 8, !dbg !24
  %vec.needs_alloc76 = icmp eq i64 %vec.current_cap74, 0, !dbg !24
  %vec.needs_grow77 = icmp eq i64 %vec.current_size73, %vec.current_cap74, !dbg !24
  %vec.needs_realloc78 = or i1 %vec.needs_alloc76, %vec.needs_grow77, !dbg !24
  br i1 %vec.needs_realloc78, label %vec.alloc79, label %vec.merge82, !dbg !24

vec.alloc79:                                      ; preds = %vec.merge59
  %8 = mul i64 %vec.current_cap74, 2, !dbg !24
  %vec.new_cap83 = select i1 %vec.needs_alloc76, i64 4, i64 %8, !dbg !24
  %vec.alloc_size84 = mul i64 %vec.new_cap83, 8, !dbg !24
  %vec.new_data85 = call ptr @malloc(i64 %vec.alloc_size84), !dbg !24
  %vec.has_data86 = icmp ne i64 %vec.current_size73, 0, !dbg !24
  br i1 %vec.has_data86, label %vec.copy80, label %vec.no_copy81, !dbg !24

vec.copy80:                                       ; preds = %vec.alloc79
  %vec.copy_size87 = mul i64 %vec.current_size73, 8, !dbg !24
  %9 = call ptr @memcpy(ptr %vec.new_data85, ptr %vec.data75, i64 %vec.copy_size87), !dbg !24
  br label %vec.no_copy81, !dbg !24

vec.no_copy81:                                    ; preds = %vec.copy80, %vec.alloc79
  store ptr %vec.new_data85, ptr %vec.data_ptr70, align 8, !dbg !24
  store i64 %vec.new_cap83, ptr %vec.cap_ptr72, align 4, !dbg !24
  br label %vec.merge82, !dbg !24

vec.merge82:                                      ; preds = %vec.no_copy81, %vec.merge59
  %vec.final_data88 = load ptr, ptr %vec.data_ptr70, align 8, !dbg !24
  %vec.reloaded_size89 = load i64, ptr %vec.size_ptr71, align 4, !dbg !24
  %vec.offset90 = mul i64 %vec.reloaded_size89, 8, !dbg !24
  %vec.element_ptr91 = getelementptr i8, ptr %vec.final_data88, i64 %vec.offset90, !dbg !24
  store i64 5, ptr %vec.element_ptr91, align 4, !dbg !24
  %vec.new_size92 = add i64 %vec.reloaded_size89, 1, !dbg !24
  store i64 %vec.new_size92, ptr %vec.size_ptr71, align 4, !dbg !24
  %vec.data_ptr93 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 0, !dbg !24
  %vec.size_ptr94 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !24
  %vec.cap_ptr95 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 2, !dbg !24
  %vec.current_size96 = load i64, ptr %vec.size_ptr94, align 4, !dbg !24
  %vec.current_cap97 = load i64, ptr %vec.cap_ptr95, align 4, !dbg !24
  %vec.data98 = load ptr, ptr %vec.data_ptr93, align 8, !dbg !24
  %vec.needs_alloc99 = icmp eq i64 %vec.current_cap97, 0, !dbg !24
  %vec.needs_grow100 = icmp eq i64 %vec.current_size96, %vec.current_cap97, !dbg !24
  %vec.needs_realloc101 = or i1 %vec.needs_alloc99, %vec.needs_grow100, !dbg !24
  br i1 %vec.needs_realloc101, label %vec.alloc102, label %vec.merge105, !dbg !24

vec.alloc102:                                     ; preds = %vec.merge82
  %10 = mul i64 %vec.current_cap97, 2, !dbg !24
  %vec.new_cap106 = select i1 %vec.needs_alloc99, i64 4, i64 %10, !dbg !24
  %vec.alloc_size107 = mul i64 %vec.new_cap106, 8, !dbg !24
  %vec.new_data108 = call ptr @malloc(i64 %vec.alloc_size107), !dbg !24
  %vec.has_data109 = icmp ne i64 %vec.current_size96, 0, !dbg !24
  br i1 %vec.has_data109, label %vec.copy103, label %vec.no_copy104, !dbg !24

vec.copy103:                                      ; preds = %vec.alloc102
  %vec.copy_size110 = mul i64 %vec.current_size96, 8, !dbg !24
  %11 = call ptr @memcpy(ptr %vec.new_data108, ptr %vec.data98, i64 %vec.copy_size110), !dbg !24
  br label %vec.no_copy104, !dbg !24

vec.no_copy104:                                   ; preds = %vec.copy103, %vec.alloc102
  store ptr %vec.new_data108, ptr %vec.data_ptr93, align 8, !dbg !24
  store i64 %vec.new_cap106, ptr %vec.cap_ptr95, align 4, !dbg !24
  br label %vec.merge105, !dbg !24

vec.merge105:                                     ; preds = %vec.no_copy104, %vec.merge82
  %vec.final_data111 = load ptr, ptr %vec.data_ptr93, align 8, !dbg !24
  %vec.reloaded_size112 = load i64, ptr %vec.size_ptr94, align 4, !dbg !24
  %vec.offset113 = mul i64 %vec.reloaded_size112, 8, !dbg !24
  %vec.element_ptr114 = getelementptr i8, ptr %vec.final_data111, i64 %vec.offset113, !dbg !24
  store i64 6, ptr %vec.element_ptr114, align 4, !dbg !24
  %vec.new_size115 = add i64 %vec.reloaded_size112, 1, !dbg !24
  store i64 %vec.new_size115, ptr %vec.size_ptr94, align 4, !dbg !24
  %vec.new116 = alloca { ptr, i64, i64 }, align 8, !dbg !24
  %vec.ptr_field117 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new116, i32 0, i32 0, !dbg !24
  store ptr null, ptr %vec.ptr_field117, align 8, !dbg !24
  %vec.size_field118 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new116, i32 0, i32 1, !dbg !24
  store i64 0, ptr %vec.size_field118, align 4, !dbg !24
  %vec.cap_field119 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new116, i32 0, i32 2, !dbg !24
  store i64 0, ptr %vec.cap_field119, align 4, !dbg !24
  %vec.new.value120 = load { ptr, i64, i64 }, ptr %vec.new116, align 8, !dbg !24
  store { ptr, i64, i64 } %vec.new.value120, ptr %evens, align 8, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %evens, metadata !11, metadata !DIExpression()), !dbg !26
  br label %for.init, !dbg !24

for.init:                                         ; preds = %vec.merge105
  store i1 true, ptr %__run_once_num, align 1, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %__run_once_num, metadata !12, metadata !DIExpression()), !dbg !27
  br label %for.cond, !dbg !24

for.cond:                                         ; preds = %for.update, %for.init
  %__run_once_num121 = load i1, ptr %__run_once_num, align 1, !dbg !24
  br i1 %__run_once_num121, label %for.body, label %for.exit, !dbg !24

for.body:                                         ; preds = %for.cond
  %vec.size_ptr122 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !24
  %vec.len = load i64, ptr %vec.size_ptr122, align 4, !dbg !24
  store i64 %vec.len, ptr %__len_num, align 4, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %__len_num, metadata !14, metadata !DIExpression()), !dbg !27
  br label %for.init123, !dbg !24

for.update:                                       ; preds = %for.exit127
  store i1 false, ptr %__run_once_num, align 1, !dbg !24
  br label %for.cond, !dbg !24

for.exit:                                         ; preds = %for.cond
  store i64 0, ptr %sum, align 4, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %sum, metadata !19, metadata !DIExpression()), !dbg !28
  br label %for.init165, !dbg !24

for.init123:                                      ; preds = %for.body
  store i64 0, ptr %__idx_num, align 4, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %__idx_num, metadata !15, metadata !DIExpression()), !dbg !27
  br label %for.cond124, !dbg !24

for.cond124:                                      ; preds = %for.update126, %for.init123
  %__idx_num128 = load i64, ptr %__idx_num, align 4, !dbg !24
  %__len_num129 = load i64, ptr %__len_num, align 4, !dbg !24
  %icmpslttmp = icmp slt i64 %__idx_num128, %__len_num129, !dbg !24
  br i1 %icmpslttmp, label %for.body125, label %for.exit127, !dbg !24

for.body125:                                      ; preds = %for.cond124
  %__idx_num130 = load i64, ptr %__idx_num, align 4, !dbg !24
  %vec.data_ptr131 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 0, !dbg !24
  %vec.size_ptr132 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !24
  %vec.data133 = load ptr, ptr %vec.data_ptr131, align 8, !dbg !24
  %vec.size = load i64, ptr %vec.size_ptr132, align 4, !dbg !24
  %vec.offset134 = mul i64 %__idx_num130, 8, !dbg !24
  %vec.element_ptr135 = getelementptr i8, ptr %vec.data133, i64 %vec.offset134, !dbg !24
  %vec.element = load i64, ptr %vec.element_ptr135, align 4, !dbg !24
  store i64 %vec.element, ptr %num, align 4, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %num, metadata !16, metadata !DIExpression()), !dbg !27
  %num136 = load i64, ptr %num, align 4, !dbg !24
  %sdivtmp = sdiv i64 %num136, 2, !dbg !24
  store i64 %sdivtmp, ptr %half, align 4, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %half, metadata !17, metadata !DIExpression()), !dbg !29
  %half137 = load i64, ptr %half, align 4, !dbg !24
  %multmp = mul i64 %half137, 2, !dbg !24
  store i64 %multmp, ptr %doubled, align 4, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %doubled, metadata !18, metadata !DIExpression()), !dbg !30
  %doubled138 = load i64, ptr %doubled, align 4, !dbg !24
  %num139 = load i64, ptr %num, align 4, !dbg !24
  %icmpeqtmp = icmp eq i64 %doubled138, %num139, !dbg !24
  br i1 %icmpeqtmp, label %then, label %ifcont, !dbg !24

for.update126:                                    ; preds = %ifcont
  %__idx_num164 = load i64, ptr %__idx_num, align 4, !dbg !24
  %addtmp = add i64 %__idx_num164, 1, !dbg !24
  store i64 %addtmp, ptr %__idx_num, align 4, !dbg !24
  br label %for.cond124, !dbg !24

for.exit127:                                      ; preds = %for.cond124
  br label %for.update, !dbg !24

then:                                             ; preds = %for.body125
  %num140 = load i64, ptr %num, align 4, !dbg !24
  %vec.data_ptr141 = getelementptr inbounds { ptr, i64, i64 }, ptr %evens, i32 0, i32 0, !dbg !24
  %vec.size_ptr142 = getelementptr inbounds { ptr, i64, i64 }, ptr %evens, i32 0, i32 1, !dbg !24
  %vec.cap_ptr143 = getelementptr inbounds { ptr, i64, i64 }, ptr %evens, i32 0, i32 2, !dbg !24
  %vec.current_size144 = load i64, ptr %vec.size_ptr142, align 4, !dbg !24
  %vec.current_cap145 = load i64, ptr %vec.cap_ptr143, align 4, !dbg !24
  %vec.data146 = load ptr, ptr %vec.data_ptr141, align 8, !dbg !24
  %vec.needs_alloc147 = icmp eq i64 %vec.current_cap145, 0, !dbg !24
  %vec.needs_grow148 = icmp eq i64 %vec.current_size144, %vec.current_cap145, !dbg !24
  %vec.needs_realloc149 = or i1 %vec.needs_alloc147, %vec.needs_grow148, !dbg !24
  br i1 %vec.needs_realloc149, label %vec.alloc150, label %vec.merge153, !dbg !24

vec.alloc150:                                     ; preds = %then
  %12 = mul i64 %vec.current_cap145, 2, !dbg !24
  %vec.new_cap154 = select i1 %vec.needs_alloc147, i64 4, i64 %12, !dbg !24
  %vec.alloc_size155 = mul i64 %vec.new_cap154, 8, !dbg !24
  %vec.new_data156 = call ptr @malloc(i64 %vec.alloc_size155), !dbg !24
  %vec.has_data157 = icmp ne i64 %vec.current_size144, 0, !dbg !24
  br i1 %vec.has_data157, label %vec.copy151, label %vec.no_copy152, !dbg !24

vec.copy151:                                      ; preds = %vec.alloc150
  %vec.copy_size158 = mul i64 %vec.current_size144, 8, !dbg !24
  %13 = call ptr @memcpy(ptr %vec.new_data156, ptr %vec.data146, i64 %vec.copy_size158), !dbg !24
  br label %vec.no_copy152, !dbg !24

vec.no_copy152:                                   ; preds = %vec.copy151, %vec.alloc150
  store ptr %vec.new_data156, ptr %vec.data_ptr141, align 8, !dbg !24
  store i64 %vec.new_cap154, ptr %vec.cap_ptr143, align 4, !dbg !24
  br label %vec.merge153, !dbg !24

vec.merge153:                                     ; preds = %vec.no_copy152, %then
  %vec.final_data159 = load ptr, ptr %vec.data_ptr141, align 8, !dbg !24
  %vec.reloaded_size160 = load i64, ptr %vec.size_ptr142, align 4, !dbg !24
  %vec.offset161 = mul i64 %vec.reloaded_size160, 8, !dbg !24
  %vec.element_ptr162 = getelementptr i8, ptr %vec.final_data159, i64 %vec.offset161, !dbg !24
  store i64 %num140, ptr %vec.element_ptr162, align 4, !dbg !24
  %vec.new_size163 = add i64 %vec.reloaded_size160, 1, !dbg !24
  store i64 %vec.new_size163, ptr %vec.size_ptr142, align 4, !dbg !24
  br label %ifcont, !dbg !24

ifcont:                                           ; preds = %vec.merge153, %for.body125
  br label %for.update126, !dbg !24

for.init165:                                      ; preds = %for.exit
  store i1 true, ptr %__run_once_even, align 1, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %__run_once_even, metadata !20, metadata !DIExpression()), !dbg !31
  br label %for.cond166, !dbg !24

for.cond166:                                      ; preds = %for.update168, %for.init165
  %__run_once_even170 = load i1, ptr %__run_once_even, align 1, !dbg !24
  br i1 %__run_once_even170, label %for.body167, label %for.exit169, !dbg !24

for.body167:                                      ; preds = %for.cond166
  %vec.size_ptr171 = getelementptr inbounds { ptr, i64, i64 }, ptr %evens, i32 0, i32 1, !dbg !24
  %vec.len172 = load i64, ptr %vec.size_ptr171, align 4, !dbg !24
  store i64 %vec.len172, ptr %__len_even, align 4, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %__len_even, metadata !21, metadata !DIExpression()), !dbg !31
  br label %for.init173, !dbg !24

for.update168:                                    ; preds = %for.exit177
  store i1 false, ptr %__run_once_even, align 1, !dbg !24
  br label %for.cond166, !dbg !24

for.exit169:                                      ; preds = %for.cond166
  %sum194 = load i64, ptr %sum, align 4, !dbg !24
  %evens_cleanup_load = load { ptr, i64, i64 }, ptr %evens, align 8, !dbg !24
  %evens_data_ptr = extractvalue { ptr, i64, i64 } %evens_cleanup_load, 0, !dbg !24
  %evens_null_check = icmp ne ptr %evens_data_ptr, null, !dbg !24
  br i1 %evens_null_check, label %evens_free_block, label %evens_continue, !dbg !24

for.init173:                                      ; preds = %for.body167
  store i64 0, ptr %__idx_even, align 4, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %__idx_even, metadata !22, metadata !DIExpression()), !dbg !31
  br label %for.cond174, !dbg !24

for.cond174:                                      ; preds = %for.update176, %for.init173
  %__idx_even178 = load i64, ptr %__idx_even, align 4, !dbg !24
  %__len_even179 = load i64, ptr %__len_even, align 4, !dbg !24
  %icmpslttmp180 = icmp slt i64 %__idx_even178, %__len_even179, !dbg !24
  br i1 %icmpslttmp180, label %for.body175, label %for.exit177, !dbg !24

for.body175:                                      ; preds = %for.cond174
  %__idx_even181 = load i64, ptr %__idx_even, align 4, !dbg !24
  %vec.data_ptr182 = getelementptr inbounds { ptr, i64, i64 }, ptr %evens, i32 0, i32 0, !dbg !24
  %vec.size_ptr183 = getelementptr inbounds { ptr, i64, i64 }, ptr %evens, i32 0, i32 1, !dbg !24
  %vec.data184 = load ptr, ptr %vec.data_ptr182, align 8, !dbg !24
  %vec.size185 = load i64, ptr %vec.size_ptr183, align 4, !dbg !24
  %vec.offset186 = mul i64 %__idx_even181, 8, !dbg !24
  %vec.element_ptr187 = getelementptr i8, ptr %vec.data184, i64 %vec.offset186, !dbg !24
  %vec.element188 = load i64, ptr %vec.element_ptr187, align 4, !dbg !24
  store i64 %vec.element188, ptr %even, align 4, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %even, metadata !23, metadata !DIExpression()), !dbg !31
  %sum189 = load i64, ptr %sum, align 4, !dbg !24
  %even190 = load i64, ptr %even, align 4, !dbg !24
  %addtmp191 = add i64 %sum189, %even190, !dbg !24
  store i64 %addtmp191, ptr %sum, align 4, !dbg !24
  br label %for.update176, !dbg !24

for.update176:                                    ; preds = %for.body175
  %__idx_even192 = load i64, ptr %__idx_even, align 4, !dbg !24
  %addtmp193 = add i64 %__idx_even192, 1, !dbg !24
  store i64 %addtmp193, ptr %__idx_even, align 4, !dbg !24
  br label %for.cond174, !dbg !24

for.exit177:                                      ; preds = %for.cond174
  br label %for.update168, !dbg !24

evens_free_block:                                 ; preds = %for.exit169
  call void @free(ptr %evens_data_ptr), !dbg !24
  br label %evens_continue, !dbg !24

evens_continue:                                   ; preds = %evens_free_block, %for.exit169
  %numbers_cleanup_load = load { ptr, i64, i64 }, ptr %numbers, align 8, !dbg !24
  %numbers_data_ptr = extractvalue { ptr, i64, i64 } %numbers_cleanup_load, 0, !dbg !24
  %numbers_null_check = icmp ne ptr %numbers_data_ptr, null, !dbg !24
  br i1 %numbers_null_check, label %numbers_free_block, label %numbers_continue, !dbg !24

numbers_free_block:                               ; preds = %evens_continue
  call void @free(ptr %numbers_data_ptr), !dbg !24
  br label %numbers_continue, !dbg !24

numbers_continue:                                 ; preds = %numbers_free_block, %evens_continue
  ret i64 %sum194, !dbg !24
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @malloc(i64)

declare ptr @memcpy(ptr, ptr, i64)

declare void @free(ptr)

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "vec_filter.vyn.ll", directory: "/home/rick/Projects/Vyn/examples")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11, !12, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23}
!9 = !DILocalVariable(name: "numbers", scope: !4, file: !1, line: 4, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_{ ptr, i64, i64 }", scope: !1, file: !1, size: 192, align: 8)
!11 = !DILocalVariable(name: "evens", scope: !4, file: !1, line: 12, type: !10)
!12 = !DILocalVariable(name: "__run_once_num", scope: !4, file: !1, line: 14, type: !13)
!13 = !DIBasicType(name: "bool", size: 1, encoding: DW_ATE_boolean)
!14 = !DILocalVariable(name: "__len_num", scope: !4, file: !1, line: 14, type: !7)
!15 = !DILocalVariable(name: "__idx_num", scope: !4, file: !1, line: 14, type: !7)
!16 = !DILocalVariable(name: "num", scope: !4, file: !1, line: 14, type: !7)
!17 = !DILocalVariable(name: "half", scope: !4, file: !1, line: 15, type: !7)
!18 = !DILocalVariable(name: "doubled", scope: !4, file: !1, line: 17, type: !7)
!19 = !DILocalVariable(name: "sum", scope: !4, file: !1, line: 23, type: !7)
!20 = !DILocalVariable(name: "__run_once_even", scope: !4, file: !1, line: 25, type: !13)
!21 = !DILocalVariable(name: "__len_even", scope: !4, file: !1, line: 25, type: !7)
!22 = !DILocalVariable(name: "__idx_even", scope: !4, file: !1, line: 25, type: !7)
!23 = !DILocalVariable(name: "even", scope: !4, file: !1, line: 25, type: !7)
!24 = !DILocation(line: 3, column: 1, scope: !4)
!25 = !DILocation(line: 4, column: 1, scope: !4)
!26 = !DILocation(line: 12, column: 1, scope: !4)
!27 = !DILocation(line: 14, column: 10, scope: !4)
!28 = !DILocation(line: 23, column: 5, scope: !4)
!29 = !DILocation(line: 15, column: 9, scope: !4)
!30 = !DILocation(line: 17, column: 1, scope: !4)
!31 = !DILocation(line: 25, column: 10, scope: !4)
