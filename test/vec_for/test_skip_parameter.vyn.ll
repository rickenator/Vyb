; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() !dbg !4 {
entry:
  %num291 = alloca i64, align 8, !dbg !30
  %__idx_num279 = alloca i64, align 8, !dbg !30
  %__len_num273 = alloca i64, align 8, !dbg !30
  %__step_num270 = alloca i64, align 8, !dbg !30
  %__run_once_num268 = alloca i1, align 1, !dbg !30
  %sum3 = alloca i64, align 8, !dbg !30
  %num256 = alloca i64, align 8, !dbg !30
  %__idx_num244 = alloca i64, align 8, !dbg !30
  %__len_num238 = alloca i64, align 8, !dbg !30
  %__step_num235 = alloca i64, align 8, !dbg !30
  %__run_once_num233 = alloca i1, align 1, !dbg !30
  %sum2 = alloca i64, align 8, !dbg !30
  %num = alloca i64, align 8, !dbg !30
  %__idx_num = alloca i64, align 8, !dbg !30
  %__len_num = alloca i64, align 8, !dbg !30
  %__step_num = alloca i64, align 8, !dbg !30
  %__run_once_num = alloca i1, align 1, !dbg !30
  %sum1 = alloca i64, align 8, !dbg !30
  %numbers = alloca { ptr, i64, i64 }, align 8, !dbg !30
  %vec.new = alloca { ptr, i64, i64 }, align 8, !dbg !30
  %vec.ptr_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 0, !dbg !30
  store ptr null, ptr %vec.ptr_field, align 8, !dbg !30
  %vec.size_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 1, !dbg !30
  store i64 0, ptr %vec.size_field, align 4, !dbg !30
  %vec.cap_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 2, !dbg !30
  store i64 0, ptr %vec.cap_field, align 4, !dbg !30
  %vec.new.value = load { ptr, i64, i64 }, ptr %vec.new, align 8, !dbg !30
  store { ptr, i64, i64 } %vec.new.value, ptr %numbers, align 8, !dbg !30
  call void @llvm.dbg.declare(metadata ptr %numbers, metadata !9, metadata !DIExpression()), !dbg !31
  %vec.data_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 0, !dbg !30
  %vec.size_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !30
  %vec.cap_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 2, !dbg !30
  %vec.current_size = load i64, ptr %vec.size_ptr, align 4, !dbg !30
  %vec.current_cap = load i64, ptr %vec.cap_ptr, align 4, !dbg !30
  %vec.data = load ptr, ptr %vec.data_ptr, align 8, !dbg !30
  %vec.needs_alloc = icmp eq i64 %vec.current_cap, 0, !dbg !30
  %vec.needs_grow = icmp eq i64 %vec.current_size, %vec.current_cap, !dbg !30
  %vec.needs_realloc = or i1 %vec.needs_alloc, %vec.needs_grow, !dbg !30
  br i1 %vec.needs_realloc, label %vec.alloc, label %vec.merge, !dbg !30

vec.alloc:                                        ; preds = %entry
  %0 = mul i64 %vec.current_cap, 2, !dbg !30
  %vec.new_cap = select i1 %vec.needs_alloc, i64 4, i64 %0, !dbg !30
  %vec.alloc_size = mul i64 %vec.new_cap, 8, !dbg !30
  %vec.new_data = call ptr @malloc(i64 %vec.alloc_size), !dbg !30
  %vec.has_data = icmp ne i64 %vec.current_size, 0, !dbg !30
  br i1 %vec.has_data, label %vec.copy, label %vec.no_copy, !dbg !30

vec.copy:                                         ; preds = %vec.alloc
  %vec.copy_size = mul i64 %vec.current_size, 8, !dbg !30
  %1 = call ptr @memcpy(ptr %vec.new_data, ptr %vec.data, i64 %vec.copy_size), !dbg !30
  br label %vec.no_copy, !dbg !30

vec.no_copy:                                      ; preds = %vec.copy, %vec.alloc
  store ptr %vec.new_data, ptr %vec.data_ptr, align 8, !dbg !30
  store i64 %vec.new_cap, ptr %vec.cap_ptr, align 4, !dbg !30
  br label %vec.merge, !dbg !30

vec.merge:                                        ; preds = %vec.no_copy, %entry
  %vec.final_data = load ptr, ptr %vec.data_ptr, align 8, !dbg !30
  %vec.reloaded_size = load i64, ptr %vec.size_ptr, align 4, !dbg !30
  %vec.offset = mul i64 %vec.reloaded_size, 8, !dbg !30
  %vec.element_ptr = getelementptr i8, ptr %vec.final_data, i64 %vec.offset, !dbg !30
  store i64 1, ptr %vec.element_ptr, align 4, !dbg !30
  %vec.new_size = add i64 %vec.reloaded_size, 1, !dbg !30
  store i64 %vec.new_size, ptr %vec.size_ptr, align 4, !dbg !30
  %vec.data_ptr1 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 0, !dbg !30
  %vec.size_ptr2 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !30
  %vec.cap_ptr3 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 2, !dbg !30
  %vec.current_size4 = load i64, ptr %vec.size_ptr2, align 4, !dbg !30
  %vec.current_cap5 = load i64, ptr %vec.cap_ptr3, align 4, !dbg !30
  %vec.data6 = load ptr, ptr %vec.data_ptr1, align 8, !dbg !30
  %vec.needs_alloc7 = icmp eq i64 %vec.current_cap5, 0, !dbg !30
  %vec.needs_grow8 = icmp eq i64 %vec.current_size4, %vec.current_cap5, !dbg !30
  %vec.needs_realloc9 = or i1 %vec.needs_alloc7, %vec.needs_grow8, !dbg !30
  br i1 %vec.needs_realloc9, label %vec.alloc10, label %vec.merge13, !dbg !30

vec.alloc10:                                      ; preds = %vec.merge
  %2 = mul i64 %vec.current_cap5, 2, !dbg !30
  %vec.new_cap14 = select i1 %vec.needs_alloc7, i64 4, i64 %2, !dbg !30
  %vec.alloc_size15 = mul i64 %vec.new_cap14, 8, !dbg !30
  %vec.new_data16 = call ptr @malloc(i64 %vec.alloc_size15), !dbg !30
  %vec.has_data17 = icmp ne i64 %vec.current_size4, 0, !dbg !30
  br i1 %vec.has_data17, label %vec.copy11, label %vec.no_copy12, !dbg !30

vec.copy11:                                       ; preds = %vec.alloc10
  %vec.copy_size18 = mul i64 %vec.current_size4, 8, !dbg !30
  %3 = call ptr @memcpy(ptr %vec.new_data16, ptr %vec.data6, i64 %vec.copy_size18), !dbg !30
  br label %vec.no_copy12, !dbg !30

vec.no_copy12:                                    ; preds = %vec.copy11, %vec.alloc10
  store ptr %vec.new_data16, ptr %vec.data_ptr1, align 8, !dbg !30
  store i64 %vec.new_cap14, ptr %vec.cap_ptr3, align 4, !dbg !30
  br label %vec.merge13, !dbg !30

vec.merge13:                                      ; preds = %vec.no_copy12, %vec.merge
  %vec.final_data19 = load ptr, ptr %vec.data_ptr1, align 8, !dbg !30
  %vec.reloaded_size20 = load i64, ptr %vec.size_ptr2, align 4, !dbg !30
  %vec.offset21 = mul i64 %vec.reloaded_size20, 8, !dbg !30
  %vec.element_ptr22 = getelementptr i8, ptr %vec.final_data19, i64 %vec.offset21, !dbg !30
  store i64 2, ptr %vec.element_ptr22, align 4, !dbg !30
  %vec.new_size23 = add i64 %vec.reloaded_size20, 1, !dbg !30
  store i64 %vec.new_size23, ptr %vec.size_ptr2, align 4, !dbg !30
  %vec.data_ptr24 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 0, !dbg !30
  %vec.size_ptr25 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !30
  %vec.cap_ptr26 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 2, !dbg !30
  %vec.current_size27 = load i64, ptr %vec.size_ptr25, align 4, !dbg !30
  %vec.current_cap28 = load i64, ptr %vec.cap_ptr26, align 4, !dbg !30
  %vec.data29 = load ptr, ptr %vec.data_ptr24, align 8, !dbg !30
  %vec.needs_alloc30 = icmp eq i64 %vec.current_cap28, 0, !dbg !30
  %vec.needs_grow31 = icmp eq i64 %vec.current_size27, %vec.current_cap28, !dbg !30
  %vec.needs_realloc32 = or i1 %vec.needs_alloc30, %vec.needs_grow31, !dbg !30
  br i1 %vec.needs_realloc32, label %vec.alloc33, label %vec.merge36, !dbg !30

vec.alloc33:                                      ; preds = %vec.merge13
  %4 = mul i64 %vec.current_cap28, 2, !dbg !30
  %vec.new_cap37 = select i1 %vec.needs_alloc30, i64 4, i64 %4, !dbg !30
  %vec.alloc_size38 = mul i64 %vec.new_cap37, 8, !dbg !30
  %vec.new_data39 = call ptr @malloc(i64 %vec.alloc_size38), !dbg !30
  %vec.has_data40 = icmp ne i64 %vec.current_size27, 0, !dbg !30
  br i1 %vec.has_data40, label %vec.copy34, label %vec.no_copy35, !dbg !30

vec.copy34:                                       ; preds = %vec.alloc33
  %vec.copy_size41 = mul i64 %vec.current_size27, 8, !dbg !30
  %5 = call ptr @memcpy(ptr %vec.new_data39, ptr %vec.data29, i64 %vec.copy_size41), !dbg !30
  br label %vec.no_copy35, !dbg !30

vec.no_copy35:                                    ; preds = %vec.copy34, %vec.alloc33
  store ptr %vec.new_data39, ptr %vec.data_ptr24, align 8, !dbg !30
  store i64 %vec.new_cap37, ptr %vec.cap_ptr26, align 4, !dbg !30
  br label %vec.merge36, !dbg !30

vec.merge36:                                      ; preds = %vec.no_copy35, %vec.merge13
  %vec.final_data42 = load ptr, ptr %vec.data_ptr24, align 8, !dbg !30
  %vec.reloaded_size43 = load i64, ptr %vec.size_ptr25, align 4, !dbg !30
  %vec.offset44 = mul i64 %vec.reloaded_size43, 8, !dbg !30
  %vec.element_ptr45 = getelementptr i8, ptr %vec.final_data42, i64 %vec.offset44, !dbg !30
  store i64 3, ptr %vec.element_ptr45, align 4, !dbg !30
  %vec.new_size46 = add i64 %vec.reloaded_size43, 1, !dbg !30
  store i64 %vec.new_size46, ptr %vec.size_ptr25, align 4, !dbg !30
  %vec.data_ptr47 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 0, !dbg !30
  %vec.size_ptr48 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !30
  %vec.cap_ptr49 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 2, !dbg !30
  %vec.current_size50 = load i64, ptr %vec.size_ptr48, align 4, !dbg !30
  %vec.current_cap51 = load i64, ptr %vec.cap_ptr49, align 4, !dbg !30
  %vec.data52 = load ptr, ptr %vec.data_ptr47, align 8, !dbg !30
  %vec.needs_alloc53 = icmp eq i64 %vec.current_cap51, 0, !dbg !30
  %vec.needs_grow54 = icmp eq i64 %vec.current_size50, %vec.current_cap51, !dbg !30
  %vec.needs_realloc55 = or i1 %vec.needs_alloc53, %vec.needs_grow54, !dbg !30
  br i1 %vec.needs_realloc55, label %vec.alloc56, label %vec.merge59, !dbg !30

vec.alloc56:                                      ; preds = %vec.merge36
  %6 = mul i64 %vec.current_cap51, 2, !dbg !30
  %vec.new_cap60 = select i1 %vec.needs_alloc53, i64 4, i64 %6, !dbg !30
  %vec.alloc_size61 = mul i64 %vec.new_cap60, 8, !dbg !30
  %vec.new_data62 = call ptr @malloc(i64 %vec.alloc_size61), !dbg !30
  %vec.has_data63 = icmp ne i64 %vec.current_size50, 0, !dbg !30
  br i1 %vec.has_data63, label %vec.copy57, label %vec.no_copy58, !dbg !30

vec.copy57:                                       ; preds = %vec.alloc56
  %vec.copy_size64 = mul i64 %vec.current_size50, 8, !dbg !30
  %7 = call ptr @memcpy(ptr %vec.new_data62, ptr %vec.data52, i64 %vec.copy_size64), !dbg !30
  br label %vec.no_copy58, !dbg !30

vec.no_copy58:                                    ; preds = %vec.copy57, %vec.alloc56
  store ptr %vec.new_data62, ptr %vec.data_ptr47, align 8, !dbg !30
  store i64 %vec.new_cap60, ptr %vec.cap_ptr49, align 4, !dbg !30
  br label %vec.merge59, !dbg !30

vec.merge59:                                      ; preds = %vec.no_copy58, %vec.merge36
  %vec.final_data65 = load ptr, ptr %vec.data_ptr47, align 8, !dbg !30
  %vec.reloaded_size66 = load i64, ptr %vec.size_ptr48, align 4, !dbg !30
  %vec.offset67 = mul i64 %vec.reloaded_size66, 8, !dbg !30
  %vec.element_ptr68 = getelementptr i8, ptr %vec.final_data65, i64 %vec.offset67, !dbg !30
  store i64 4, ptr %vec.element_ptr68, align 4, !dbg !30
  %vec.new_size69 = add i64 %vec.reloaded_size66, 1, !dbg !30
  store i64 %vec.new_size69, ptr %vec.size_ptr48, align 4, !dbg !30
  %vec.data_ptr70 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 0, !dbg !30
  %vec.size_ptr71 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !30
  %vec.cap_ptr72 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 2, !dbg !30
  %vec.current_size73 = load i64, ptr %vec.size_ptr71, align 4, !dbg !30
  %vec.current_cap74 = load i64, ptr %vec.cap_ptr72, align 4, !dbg !30
  %vec.data75 = load ptr, ptr %vec.data_ptr70, align 8, !dbg !30
  %vec.needs_alloc76 = icmp eq i64 %vec.current_cap74, 0, !dbg !30
  %vec.needs_grow77 = icmp eq i64 %vec.current_size73, %vec.current_cap74, !dbg !30
  %vec.needs_realloc78 = or i1 %vec.needs_alloc76, %vec.needs_grow77, !dbg !30
  br i1 %vec.needs_realloc78, label %vec.alloc79, label %vec.merge82, !dbg !30

vec.alloc79:                                      ; preds = %vec.merge59
  %8 = mul i64 %vec.current_cap74, 2, !dbg !30
  %vec.new_cap83 = select i1 %vec.needs_alloc76, i64 4, i64 %8, !dbg !30
  %vec.alloc_size84 = mul i64 %vec.new_cap83, 8, !dbg !30
  %vec.new_data85 = call ptr @malloc(i64 %vec.alloc_size84), !dbg !30
  %vec.has_data86 = icmp ne i64 %vec.current_size73, 0, !dbg !30
  br i1 %vec.has_data86, label %vec.copy80, label %vec.no_copy81, !dbg !30

vec.copy80:                                       ; preds = %vec.alloc79
  %vec.copy_size87 = mul i64 %vec.current_size73, 8, !dbg !30
  %9 = call ptr @memcpy(ptr %vec.new_data85, ptr %vec.data75, i64 %vec.copy_size87), !dbg !30
  br label %vec.no_copy81, !dbg !30

vec.no_copy81:                                    ; preds = %vec.copy80, %vec.alloc79
  store ptr %vec.new_data85, ptr %vec.data_ptr70, align 8, !dbg !30
  store i64 %vec.new_cap83, ptr %vec.cap_ptr72, align 4, !dbg !30
  br label %vec.merge82, !dbg !30

vec.merge82:                                      ; preds = %vec.no_copy81, %vec.merge59
  %vec.final_data88 = load ptr, ptr %vec.data_ptr70, align 8, !dbg !30
  %vec.reloaded_size89 = load i64, ptr %vec.size_ptr71, align 4, !dbg !30
  %vec.offset90 = mul i64 %vec.reloaded_size89, 8, !dbg !30
  %vec.element_ptr91 = getelementptr i8, ptr %vec.final_data88, i64 %vec.offset90, !dbg !30
  store i64 5, ptr %vec.element_ptr91, align 4, !dbg !30
  %vec.new_size92 = add i64 %vec.reloaded_size89, 1, !dbg !30
  store i64 %vec.new_size92, ptr %vec.size_ptr71, align 4, !dbg !30
  %vec.data_ptr93 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 0, !dbg !30
  %vec.size_ptr94 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !30
  %vec.cap_ptr95 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 2, !dbg !30
  %vec.current_size96 = load i64, ptr %vec.size_ptr94, align 4, !dbg !30
  %vec.current_cap97 = load i64, ptr %vec.cap_ptr95, align 4, !dbg !30
  %vec.data98 = load ptr, ptr %vec.data_ptr93, align 8, !dbg !30
  %vec.needs_alloc99 = icmp eq i64 %vec.current_cap97, 0, !dbg !30
  %vec.needs_grow100 = icmp eq i64 %vec.current_size96, %vec.current_cap97, !dbg !30
  %vec.needs_realloc101 = or i1 %vec.needs_alloc99, %vec.needs_grow100, !dbg !30
  br i1 %vec.needs_realloc101, label %vec.alloc102, label %vec.merge105, !dbg !30

vec.alloc102:                                     ; preds = %vec.merge82
  %10 = mul i64 %vec.current_cap97, 2, !dbg !30
  %vec.new_cap106 = select i1 %vec.needs_alloc99, i64 4, i64 %10, !dbg !30
  %vec.alloc_size107 = mul i64 %vec.new_cap106, 8, !dbg !30
  %vec.new_data108 = call ptr @malloc(i64 %vec.alloc_size107), !dbg !30
  %vec.has_data109 = icmp ne i64 %vec.current_size96, 0, !dbg !30
  br i1 %vec.has_data109, label %vec.copy103, label %vec.no_copy104, !dbg !30

vec.copy103:                                      ; preds = %vec.alloc102
  %vec.copy_size110 = mul i64 %vec.current_size96, 8, !dbg !30
  %11 = call ptr @memcpy(ptr %vec.new_data108, ptr %vec.data98, i64 %vec.copy_size110), !dbg !30
  br label %vec.no_copy104, !dbg !30

vec.no_copy104:                                   ; preds = %vec.copy103, %vec.alloc102
  store ptr %vec.new_data108, ptr %vec.data_ptr93, align 8, !dbg !30
  store i64 %vec.new_cap106, ptr %vec.cap_ptr95, align 4, !dbg !30
  br label %vec.merge105, !dbg !30

vec.merge105:                                     ; preds = %vec.no_copy104, %vec.merge82
  %vec.final_data111 = load ptr, ptr %vec.data_ptr93, align 8, !dbg !30
  %vec.reloaded_size112 = load i64, ptr %vec.size_ptr94, align 4, !dbg !30
  %vec.offset113 = mul i64 %vec.reloaded_size112, 8, !dbg !30
  %vec.element_ptr114 = getelementptr i8, ptr %vec.final_data111, i64 %vec.offset113, !dbg !30
  store i64 6, ptr %vec.element_ptr114, align 4, !dbg !30
  %vec.new_size115 = add i64 %vec.reloaded_size112, 1, !dbg !30
  store i64 %vec.new_size115, ptr %vec.size_ptr94, align 4, !dbg !30
  %vec.data_ptr116 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 0, !dbg !30
  %vec.size_ptr117 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !30
  %vec.cap_ptr118 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 2, !dbg !30
  %vec.current_size119 = load i64, ptr %vec.size_ptr117, align 4, !dbg !30
  %vec.current_cap120 = load i64, ptr %vec.cap_ptr118, align 4, !dbg !30
  %vec.data121 = load ptr, ptr %vec.data_ptr116, align 8, !dbg !30
  %vec.needs_alloc122 = icmp eq i64 %vec.current_cap120, 0, !dbg !30
  %vec.needs_grow123 = icmp eq i64 %vec.current_size119, %vec.current_cap120, !dbg !30
  %vec.needs_realloc124 = or i1 %vec.needs_alloc122, %vec.needs_grow123, !dbg !30
  br i1 %vec.needs_realloc124, label %vec.alloc125, label %vec.merge128, !dbg !30

vec.alloc125:                                     ; preds = %vec.merge105
  %12 = mul i64 %vec.current_cap120, 2, !dbg !30
  %vec.new_cap129 = select i1 %vec.needs_alloc122, i64 4, i64 %12, !dbg !30
  %vec.alloc_size130 = mul i64 %vec.new_cap129, 8, !dbg !30
  %vec.new_data131 = call ptr @malloc(i64 %vec.alloc_size130), !dbg !30
  %vec.has_data132 = icmp ne i64 %vec.current_size119, 0, !dbg !30
  br i1 %vec.has_data132, label %vec.copy126, label %vec.no_copy127, !dbg !30

vec.copy126:                                      ; preds = %vec.alloc125
  %vec.copy_size133 = mul i64 %vec.current_size119, 8, !dbg !30
  %13 = call ptr @memcpy(ptr %vec.new_data131, ptr %vec.data121, i64 %vec.copy_size133), !dbg !30
  br label %vec.no_copy127, !dbg !30

vec.no_copy127:                                   ; preds = %vec.copy126, %vec.alloc125
  store ptr %vec.new_data131, ptr %vec.data_ptr116, align 8, !dbg !30
  store i64 %vec.new_cap129, ptr %vec.cap_ptr118, align 4, !dbg !30
  br label %vec.merge128, !dbg !30

vec.merge128:                                     ; preds = %vec.no_copy127, %vec.merge105
  %vec.final_data134 = load ptr, ptr %vec.data_ptr116, align 8, !dbg !30
  %vec.reloaded_size135 = load i64, ptr %vec.size_ptr117, align 4, !dbg !30
  %vec.offset136 = mul i64 %vec.reloaded_size135, 8, !dbg !30
  %vec.element_ptr137 = getelementptr i8, ptr %vec.final_data134, i64 %vec.offset136, !dbg !30
  store i64 7, ptr %vec.element_ptr137, align 4, !dbg !30
  %vec.new_size138 = add i64 %vec.reloaded_size135, 1, !dbg !30
  store i64 %vec.new_size138, ptr %vec.size_ptr117, align 4, !dbg !30
  %vec.data_ptr139 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 0, !dbg !30
  %vec.size_ptr140 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !30
  %vec.cap_ptr141 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 2, !dbg !30
  %vec.current_size142 = load i64, ptr %vec.size_ptr140, align 4, !dbg !30
  %vec.current_cap143 = load i64, ptr %vec.cap_ptr141, align 4, !dbg !30
  %vec.data144 = load ptr, ptr %vec.data_ptr139, align 8, !dbg !30
  %vec.needs_alloc145 = icmp eq i64 %vec.current_cap143, 0, !dbg !30
  %vec.needs_grow146 = icmp eq i64 %vec.current_size142, %vec.current_cap143, !dbg !30
  %vec.needs_realloc147 = or i1 %vec.needs_alloc145, %vec.needs_grow146, !dbg !30
  br i1 %vec.needs_realloc147, label %vec.alloc148, label %vec.merge151, !dbg !30

vec.alloc148:                                     ; preds = %vec.merge128
  %14 = mul i64 %vec.current_cap143, 2, !dbg !30
  %vec.new_cap152 = select i1 %vec.needs_alloc145, i64 4, i64 %14, !dbg !30
  %vec.alloc_size153 = mul i64 %vec.new_cap152, 8, !dbg !30
  %vec.new_data154 = call ptr @malloc(i64 %vec.alloc_size153), !dbg !30
  %vec.has_data155 = icmp ne i64 %vec.current_size142, 0, !dbg !30
  br i1 %vec.has_data155, label %vec.copy149, label %vec.no_copy150, !dbg !30

vec.copy149:                                      ; preds = %vec.alloc148
  %vec.copy_size156 = mul i64 %vec.current_size142, 8, !dbg !30
  %15 = call ptr @memcpy(ptr %vec.new_data154, ptr %vec.data144, i64 %vec.copy_size156), !dbg !30
  br label %vec.no_copy150, !dbg !30

vec.no_copy150:                                   ; preds = %vec.copy149, %vec.alloc148
  store ptr %vec.new_data154, ptr %vec.data_ptr139, align 8, !dbg !30
  store i64 %vec.new_cap152, ptr %vec.cap_ptr141, align 4, !dbg !30
  br label %vec.merge151, !dbg !30

vec.merge151:                                     ; preds = %vec.no_copy150, %vec.merge128
  %vec.final_data157 = load ptr, ptr %vec.data_ptr139, align 8, !dbg !30
  %vec.reloaded_size158 = load i64, ptr %vec.size_ptr140, align 4, !dbg !30
  %vec.offset159 = mul i64 %vec.reloaded_size158, 8, !dbg !30
  %vec.element_ptr160 = getelementptr i8, ptr %vec.final_data157, i64 %vec.offset159, !dbg !30
  store i64 8, ptr %vec.element_ptr160, align 4, !dbg !30
  %vec.new_size161 = add i64 %vec.reloaded_size158, 1, !dbg !30
  store i64 %vec.new_size161, ptr %vec.size_ptr140, align 4, !dbg !30
  %vec.data_ptr162 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 0, !dbg !30
  %vec.size_ptr163 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !30
  %vec.cap_ptr164 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 2, !dbg !30
  %vec.current_size165 = load i64, ptr %vec.size_ptr163, align 4, !dbg !30
  %vec.current_cap166 = load i64, ptr %vec.cap_ptr164, align 4, !dbg !30
  %vec.data167 = load ptr, ptr %vec.data_ptr162, align 8, !dbg !30
  %vec.needs_alloc168 = icmp eq i64 %vec.current_cap166, 0, !dbg !30
  %vec.needs_grow169 = icmp eq i64 %vec.current_size165, %vec.current_cap166, !dbg !30
  %vec.needs_realloc170 = or i1 %vec.needs_alloc168, %vec.needs_grow169, !dbg !30
  br i1 %vec.needs_realloc170, label %vec.alloc171, label %vec.merge174, !dbg !30

vec.alloc171:                                     ; preds = %vec.merge151
  %16 = mul i64 %vec.current_cap166, 2, !dbg !30
  %vec.new_cap175 = select i1 %vec.needs_alloc168, i64 4, i64 %16, !dbg !30
  %vec.alloc_size176 = mul i64 %vec.new_cap175, 8, !dbg !30
  %vec.new_data177 = call ptr @malloc(i64 %vec.alloc_size176), !dbg !30
  %vec.has_data178 = icmp ne i64 %vec.current_size165, 0, !dbg !30
  br i1 %vec.has_data178, label %vec.copy172, label %vec.no_copy173, !dbg !30

vec.copy172:                                      ; preds = %vec.alloc171
  %vec.copy_size179 = mul i64 %vec.current_size165, 8, !dbg !30
  %17 = call ptr @memcpy(ptr %vec.new_data177, ptr %vec.data167, i64 %vec.copy_size179), !dbg !30
  br label %vec.no_copy173, !dbg !30

vec.no_copy173:                                   ; preds = %vec.copy172, %vec.alloc171
  store ptr %vec.new_data177, ptr %vec.data_ptr162, align 8, !dbg !30
  store i64 %vec.new_cap175, ptr %vec.cap_ptr164, align 4, !dbg !30
  br label %vec.merge174, !dbg !30

vec.merge174:                                     ; preds = %vec.no_copy173, %vec.merge151
  %vec.final_data180 = load ptr, ptr %vec.data_ptr162, align 8, !dbg !30
  %vec.reloaded_size181 = load i64, ptr %vec.size_ptr163, align 4, !dbg !30
  %vec.offset182 = mul i64 %vec.reloaded_size181, 8, !dbg !30
  %vec.element_ptr183 = getelementptr i8, ptr %vec.final_data180, i64 %vec.offset182, !dbg !30
  store i64 9, ptr %vec.element_ptr183, align 4, !dbg !30
  %vec.new_size184 = add i64 %vec.reloaded_size181, 1, !dbg !30
  store i64 %vec.new_size184, ptr %vec.size_ptr163, align 4, !dbg !30
  %vec.data_ptr185 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 0, !dbg !30
  %vec.size_ptr186 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !30
  %vec.cap_ptr187 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 2, !dbg !30
  %vec.current_size188 = load i64, ptr %vec.size_ptr186, align 4, !dbg !30
  %vec.current_cap189 = load i64, ptr %vec.cap_ptr187, align 4, !dbg !30
  %vec.data190 = load ptr, ptr %vec.data_ptr185, align 8, !dbg !30
  %vec.needs_alloc191 = icmp eq i64 %vec.current_cap189, 0, !dbg !30
  %vec.needs_grow192 = icmp eq i64 %vec.current_size188, %vec.current_cap189, !dbg !30
  %vec.needs_realloc193 = or i1 %vec.needs_alloc191, %vec.needs_grow192, !dbg !30
  br i1 %vec.needs_realloc193, label %vec.alloc194, label %vec.merge197, !dbg !30

vec.alloc194:                                     ; preds = %vec.merge174
  %18 = mul i64 %vec.current_cap189, 2, !dbg !30
  %vec.new_cap198 = select i1 %vec.needs_alloc191, i64 4, i64 %18, !dbg !30
  %vec.alloc_size199 = mul i64 %vec.new_cap198, 8, !dbg !30
  %vec.new_data200 = call ptr @malloc(i64 %vec.alloc_size199), !dbg !30
  %vec.has_data201 = icmp ne i64 %vec.current_size188, 0, !dbg !30
  br i1 %vec.has_data201, label %vec.copy195, label %vec.no_copy196, !dbg !30

vec.copy195:                                      ; preds = %vec.alloc194
  %vec.copy_size202 = mul i64 %vec.current_size188, 8, !dbg !30
  %19 = call ptr @memcpy(ptr %vec.new_data200, ptr %vec.data190, i64 %vec.copy_size202), !dbg !30
  br label %vec.no_copy196, !dbg !30

vec.no_copy196:                                   ; preds = %vec.copy195, %vec.alloc194
  store ptr %vec.new_data200, ptr %vec.data_ptr185, align 8, !dbg !30
  store i64 %vec.new_cap198, ptr %vec.cap_ptr187, align 4, !dbg !30
  br label %vec.merge197, !dbg !30

vec.merge197:                                     ; preds = %vec.no_copy196, %vec.merge174
  %vec.final_data203 = load ptr, ptr %vec.data_ptr185, align 8, !dbg !30
  %vec.reloaded_size204 = load i64, ptr %vec.size_ptr186, align 4, !dbg !30
  %vec.offset205 = mul i64 %vec.reloaded_size204, 8, !dbg !30
  %vec.element_ptr206 = getelementptr i8, ptr %vec.final_data203, i64 %vec.offset205, !dbg !30
  store i64 10, ptr %vec.element_ptr206, align 4, !dbg !30
  %vec.new_size207 = add i64 %vec.reloaded_size204, 1, !dbg !30
  store i64 %vec.new_size207, ptr %vec.size_ptr186, align 4, !dbg !30
  store i64 0, ptr %sum1, align 4, !dbg !30
  call void @llvm.dbg.declare(metadata ptr %sum1, metadata !11, metadata !DIExpression()), !dbg !32
  br label %for.init, !dbg !30

for.init:                                         ; preds = %vec.merge197
  store i1 true, ptr %__run_once_num, align 1, !dbg !30
  call void @llvm.dbg.declare(metadata ptr %__run_once_num, metadata !12, metadata !DIExpression()), !dbg !33
  br label %for.cond, !dbg !30

for.cond:                                         ; preds = %for.update, %for.init
  %__run_once_num208 = load i1, ptr %__run_once_num, align 1, !dbg !30
  br i1 %__run_once_num208, label %for.body, label %for.exit, !dbg !30

for.body:                                         ; preds = %for.cond
  store i64 2, ptr %__step_num, align 4, !dbg !30
  call void @llvm.dbg.declare(metadata ptr %__step_num, metadata !14, metadata !DIExpression()), !dbg !33
  %vec.size_ptr209 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !30
  %vec.len = load i64, ptr %vec.size_ptr209, align 4, !dbg !30
  store i64 %vec.len, ptr %__len_num, align 4, !dbg !30
  call void @llvm.dbg.declare(metadata ptr %__len_num, metadata !15, metadata !DIExpression()), !dbg !33
  br label %for.init210, !dbg !30

for.update:                                       ; preds = %for.exit214
  store i1 false, ptr %__run_once_num, align 1, !dbg !30
  br label %for.cond, !dbg !30

for.exit:                                         ; preds = %for.cond
  store i64 0, ptr %sum2, align 4, !dbg !30
  call void @llvm.dbg.declare(metadata ptr %sum2, metadata !18, metadata !DIExpression()), !dbg !34
  br label %for.init228, !dbg !30

for.init210:                                      ; preds = %for.body
  store i64 0, ptr %__idx_num, align 4, !dbg !30
  call void @llvm.dbg.declare(metadata ptr %__idx_num, metadata !16, metadata !DIExpression()), !dbg !33
  br label %for.cond211, !dbg !30

for.cond211:                                      ; preds = %for.update213, %for.init210
  %__idx_num215 = load i64, ptr %__idx_num, align 4, !dbg !30
  %__len_num216 = load i64, ptr %__len_num, align 4, !dbg !30
  %icmpslttmp = icmp slt i64 %__idx_num215, %__len_num216, !dbg !30
  br i1 %icmpslttmp, label %for.body212, label %for.exit214, !dbg !30

for.body212:                                      ; preds = %for.cond211
  %__idx_num217 = load i64, ptr %__idx_num, align 4, !dbg !30
  %vec.data_ptr218 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 0, !dbg !30
  %vec.size_ptr219 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !30
  %vec.data220 = load ptr, ptr %vec.data_ptr218, align 8, !dbg !30
  %vec.size = load i64, ptr %vec.size_ptr219, align 4, !dbg !30
  %vec.offset221 = mul i64 %__idx_num217, 8, !dbg !30
  %vec.element_ptr222 = getelementptr i8, ptr %vec.data220, i64 %vec.offset221, !dbg !30
  %vec.element = load i64, ptr %vec.element_ptr222, align 4, !dbg !30
  store i64 %vec.element, ptr %num, align 4, !dbg !30
  call void @llvm.dbg.declare(metadata ptr %num, metadata !17, metadata !DIExpression()), !dbg !33
  %sum1223 = load i64, ptr %sum1, align 4, !dbg !30
  %num224 = load i64, ptr %num, align 4, !dbg !30
  %addtmp = add i64 %sum1223, %num224, !dbg !30
  store i64 %addtmp, ptr %sum1, align 4, !dbg !30
  br label %for.update213, !dbg !30

for.update213:                                    ; preds = %for.body212
  %__idx_num225 = load i64, ptr %__idx_num, align 4, !dbg !30
  %__step_num226 = load i64, ptr %__step_num, align 4, !dbg !30
  %addtmp227 = add i64 %__idx_num225, %__step_num226, !dbg !30
  store i64 %addtmp227, ptr %__idx_num, align 4, !dbg !30
  br label %for.cond211, !dbg !30

for.exit214:                                      ; preds = %for.cond211
  br label %for.update, !dbg !30

for.init228:                                      ; preds = %for.exit
  store i1 true, ptr %__run_once_num233, align 1, !dbg !30
  call void @llvm.dbg.declare(metadata ptr %__run_once_num233, metadata !19, metadata !DIExpression()), !dbg !35
  br label %for.cond229, !dbg !30

for.cond229:                                      ; preds = %for.update231, %for.init228
  %__run_once_num234 = load i1, ptr %__run_once_num233, align 1, !dbg !30
  br i1 %__run_once_num234, label %for.body230, label %for.exit232, !dbg !30

for.body230:                                      ; preds = %for.cond229
  store i64 3, ptr %__step_num235, align 4, !dbg !30
  call void @llvm.dbg.declare(metadata ptr %__step_num235, metadata !20, metadata !DIExpression()), !dbg !35
  %vec.size_ptr236 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !30
  %vec.len237 = load i64, ptr %vec.size_ptr236, align 4, !dbg !30
  store i64 %vec.len237, ptr %__len_num238, align 4, !dbg !30
  call void @llvm.dbg.declare(metadata ptr %__len_num238, metadata !21, metadata !DIExpression()), !dbg !35
  br label %for.init239, !dbg !30

for.update231:                                    ; preds = %for.exit243
  store i1 false, ptr %__run_once_num233, align 1, !dbg !30
  br label %for.cond229, !dbg !30

for.exit232:                                      ; preds = %for.cond229
  store i64 0, ptr %sum3, align 4, !dbg !30
  call void @llvm.dbg.declare(metadata ptr %sum3, metadata !24, metadata !DIExpression()), !dbg !36
  br label %for.init263, !dbg !30

for.init239:                                      ; preds = %for.body230
  store i64 0, ptr %__idx_num244, align 4, !dbg !30
  call void @llvm.dbg.declare(metadata ptr %__idx_num244, metadata !22, metadata !DIExpression()), !dbg !35
  br label %for.cond240, !dbg !30

for.cond240:                                      ; preds = %for.update242, %for.init239
  %__idx_num245 = load i64, ptr %__idx_num244, align 4, !dbg !30
  %__len_num246 = load i64, ptr %__len_num238, align 4, !dbg !30
  %icmpslttmp247 = icmp slt i64 %__idx_num245, %__len_num246, !dbg !30
  br i1 %icmpslttmp247, label %for.body241, label %for.exit243, !dbg !30

for.body241:                                      ; preds = %for.cond240
  %__idx_num248 = load i64, ptr %__idx_num244, align 4, !dbg !30
  %vec.data_ptr249 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 0, !dbg !30
  %vec.size_ptr250 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !30
  %vec.data251 = load ptr, ptr %vec.data_ptr249, align 8, !dbg !30
  %vec.size252 = load i64, ptr %vec.size_ptr250, align 4, !dbg !30
  %vec.offset253 = mul i64 %__idx_num248, 8, !dbg !30
  %vec.element_ptr254 = getelementptr i8, ptr %vec.data251, i64 %vec.offset253, !dbg !30
  %vec.element255 = load i64, ptr %vec.element_ptr254, align 4, !dbg !30
  store i64 %vec.element255, ptr %num256, align 4, !dbg !30
  call void @llvm.dbg.declare(metadata ptr %num256, metadata !23, metadata !DIExpression()), !dbg !35
  %sum2257 = load i64, ptr %sum2, align 4, !dbg !30
  %num258 = load i64, ptr %num256, align 4, !dbg !30
  %addtmp259 = add i64 %sum2257, %num258, !dbg !30
  store i64 %addtmp259, ptr %sum2, align 4, !dbg !30
  br label %for.update242, !dbg !30

for.update242:                                    ; preds = %for.body241
  %__idx_num260 = load i64, ptr %__idx_num244, align 4, !dbg !30
  %__step_num261 = load i64, ptr %__step_num235, align 4, !dbg !30
  %addtmp262 = add i64 %__idx_num260, %__step_num261, !dbg !30
  store i64 %addtmp262, ptr %__idx_num244, align 4, !dbg !30
  br label %for.cond240, !dbg !30

for.exit243:                                      ; preds = %for.cond240
  br label %for.update231, !dbg !30

for.init263:                                      ; preds = %for.exit232
  store i1 true, ptr %__run_once_num268, align 1, !dbg !30
  call void @llvm.dbg.declare(metadata ptr %__run_once_num268, metadata !25, metadata !DIExpression()), !dbg !37
  br label %for.cond264, !dbg !30

for.cond264:                                      ; preds = %for.update266, %for.init263
  %__run_once_num269 = load i1, ptr %__run_once_num268, align 1, !dbg !30
  br i1 %__run_once_num269, label %for.body265, label %for.exit267, !dbg !30

for.body265:                                      ; preds = %for.cond264
  store i64 1, ptr %__step_num270, align 4, !dbg !30
  call void @llvm.dbg.declare(metadata ptr %__step_num270, metadata !26, metadata !DIExpression()), !dbg !37
  %vec.size_ptr271 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !30
  %vec.len272 = load i64, ptr %vec.size_ptr271, align 4, !dbg !30
  store i64 %vec.len272, ptr %__len_num273, align 4, !dbg !30
  call void @llvm.dbg.declare(metadata ptr %__len_num273, metadata !27, metadata !DIExpression()), !dbg !37
  br label %for.init274, !dbg !30

for.update266:                                    ; preds = %for.exit278
  store i1 false, ptr %__run_once_num268, align 1, !dbg !30
  br label %for.cond264, !dbg !30

for.exit267:                                      ; preds = %for.cond264
  %sum1298 = load i64, ptr %sum1, align 4, !dbg !30
  %icmpneqtmp = icmp ne i64 %sum1298, 25, !dbg !30
  br i1 %icmpneqtmp, label %then, label %ifcont, !dbg !30

for.init274:                                      ; preds = %for.body265
  store i64 0, ptr %__idx_num279, align 4, !dbg !30
  call void @llvm.dbg.declare(metadata ptr %__idx_num279, metadata !28, metadata !DIExpression()), !dbg !37
  br label %for.cond275, !dbg !30

for.cond275:                                      ; preds = %for.update277, %for.init274
  %__idx_num280 = load i64, ptr %__idx_num279, align 4, !dbg !30
  %__len_num281 = load i64, ptr %__len_num273, align 4, !dbg !30
  %icmpslttmp282 = icmp slt i64 %__idx_num280, %__len_num281, !dbg !30
  br i1 %icmpslttmp282, label %for.body276, label %for.exit278, !dbg !30

for.body276:                                      ; preds = %for.cond275
  %__idx_num283 = load i64, ptr %__idx_num279, align 4, !dbg !30
  %vec.data_ptr284 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 0, !dbg !30
  %vec.size_ptr285 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !30
  %vec.data286 = load ptr, ptr %vec.data_ptr284, align 8, !dbg !30
  %vec.size287 = load i64, ptr %vec.size_ptr285, align 4, !dbg !30
  %vec.offset288 = mul i64 %__idx_num283, 8, !dbg !30
  %vec.element_ptr289 = getelementptr i8, ptr %vec.data286, i64 %vec.offset288, !dbg !30
  %vec.element290 = load i64, ptr %vec.element_ptr289, align 4, !dbg !30
  store i64 %vec.element290, ptr %num291, align 4, !dbg !30
  call void @llvm.dbg.declare(metadata ptr %num291, metadata !29, metadata !DIExpression()), !dbg !37
  %sum3292 = load i64, ptr %sum3, align 4, !dbg !30
  %num293 = load i64, ptr %num291, align 4, !dbg !30
  %addtmp294 = add i64 %sum3292, %num293, !dbg !30
  store i64 %addtmp294, ptr %sum3, align 4, !dbg !30
  br label %for.update277, !dbg !30

for.update277:                                    ; preds = %for.body276
  %__idx_num295 = load i64, ptr %__idx_num279, align 4, !dbg !30
  %__step_num296 = load i64, ptr %__step_num270, align 4, !dbg !30
  %addtmp297 = add i64 %__idx_num295, %__step_num296, !dbg !30
  store i64 %addtmp297, ptr %__idx_num279, align 4, !dbg !30
  br label %for.cond275, !dbg !30

for.exit278:                                      ; preds = %for.cond275
  br label %for.update266, !dbg !30

then:                                             ; preds = %for.exit267
  ret i64 1, !dbg !30

ifcont:                                           ; preds = %for.exit267
  %sum2299 = load i64, ptr %sum2, align 4, !dbg !30
  %icmpneqtmp300 = icmp ne i64 %sum2299, 22, !dbg !30
  br i1 %icmpneqtmp300, label %then301, label %ifcont302, !dbg !30

then301:                                          ; preds = %ifcont
  ret i64 2, !dbg !30

ifcont302:                                        ; preds = %ifcont
  %sum3303 = load i64, ptr %sum3, align 4, !dbg !30
  %icmpneqtmp304 = icmp ne i64 %sum3303, 55, !dbg !30
  br i1 %icmpneqtmp304, label %then305, label %ifcont306, !dbg !30

then305:                                          ; preds = %ifcont302
  ret i64 3, !dbg !30

ifcont306:                                        ; preds = %ifcont302
  ret i64 0, !dbg !30
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
!1 = !DIFile(filename: "test_skip_parameter.vyn.ll", directory: "/home/rick/Projects/Vyn/test/vec_for")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11, !12, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29}
!9 = !DILocalVariable(name: "numbers", scope: !4, file: !1, line: 4, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_{ ptr, i64, i64 }", scope: !1, file: !1, size: 192, align: 8)
!11 = !DILocalVariable(name: "sum1", scope: !4, file: !1, line: 9, type: !7)
!12 = !DILocalVariable(name: "__run_once_num", scope: !4, file: !1, line: 11, type: !13)
!13 = !DIBasicType(name: "bool", size: 1, encoding: DW_ATE_boolean)
!14 = !DILocalVariable(name: "__step_num", scope: !4, file: !1, line: 11, type: !7)
!15 = !DILocalVariable(name: "__len_num", scope: !4, file: !1, line: 11, type: !7)
!16 = !DILocalVariable(name: "__idx_num", scope: !4, file: !1, line: 11, type: !7)
!17 = !DILocalVariable(name: "num", scope: !4, file: !1, line: 11, type: !7)
!18 = !DILocalVariable(name: "sum2", scope: !4, file: !1, line: 14, type: !7)
!19 = !DILocalVariable(name: "__run_once_num", scope: !4, file: !1, line: 18, type: !13)
!20 = !DILocalVariable(name: "__step_num", scope: !4, file: !1, line: 18, type: !7)
!21 = !DILocalVariable(name: "__len_num", scope: !4, file: !1, line: 18, type: !7)
!22 = !DILocalVariable(name: "__idx_num", scope: !4, file: !1, line: 18, type: !7)
!23 = !DILocalVariable(name: "num", scope: !4, file: !1, line: 18, type: !7)
!24 = !DILocalVariable(name: "sum3", scope: !4, file: !1, line: 21, type: !7)
!25 = !DILocalVariable(name: "__run_once_num", scope: !4, file: !1, line: 25, type: !13)
!26 = !DILocalVariable(name: "__step_num", scope: !4, file: !1, line: 25, type: !7)
!27 = !DILocalVariable(name: "__len_num", scope: !4, file: !1, line: 25, type: !7)
!28 = !DILocalVariable(name: "__idx_num", scope: !4, file: !1, line: 25, type: !7)
!29 = !DILocalVariable(name: "num", scope: !4, file: !1, line: 25, type: !7)
!30 = !DILocation(line: 3, column: 1, scope: !4)
!31 = !DILocation(line: 4, column: 5, scope: !4)
!32 = !DILocation(line: 9, column: 5, scope: !4)
!33 = !DILocation(line: 11, column: 10, scope: !4)
!34 = !DILocation(line: 14, column: 5, scope: !4)
!35 = !DILocation(line: 18, column: 10, scope: !4)
!36 = !DILocation(line: 21, column: 5, scope: !4)
!37 = !DILocation(line: 25, column: 10, scope: !4)
