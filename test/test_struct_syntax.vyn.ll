; ModuleID = 'VynModule'
source_filename = "VynModule"

%Point = type { i64, i64 }
%Rectangle = type { i64, i64 }
%Person = type { i64, ptr, double }
%Company = type { ptr, i64, i1 }

@0 = private unnamed_addr constant [6 x i8] c"Alice\00", align 1
@1 = private unnamed_addr constant [9 x i8] c"Acme Inc\00", align 1
@field_name = private unnamed_addr constant [7 x i8] c"Person\00", align 1
@field_name.1 = private unnamed_addr constant [7 x i8] c"Person\00", align 1
@typename = private unnamed_addr constant [22 x i8] c"{ %Person, %Company }\00", align 1

define { %Point, %Rectangle } @createShapes() {
entry:
  %r = alloca %Rectangle, align 8
  %p = alloca %Point, align 8
  %Point_obj = alloca %Point, align 8
  %x_ptr = getelementptr inbounds %Point, ptr %Point_obj, i32 0, i32 0
  store i64 10, ptr %x_ptr, align 4
  %y_ptr = getelementptr inbounds %Point, ptr %Point_obj, i32 0, i32 1
  store i64 20, ptr %y_ptr, align 4
  %Point_value = load %Point, ptr %Point_obj, align 4
  store %Point %Point_value, ptr %p, align 4
  %Rectangle_obj = alloca %Rectangle, align 8
  %width_ptr = getelementptr inbounds %Rectangle, ptr %Rectangle_obj, i32 0, i32 0
  store i64 100, ptr %width_ptr, align 4
  %height_ptr = getelementptr inbounds %Rectangle, ptr %Rectangle_obj, i32 0, i32 1
  store i64 50, ptr %height_ptr, align 4
  %Rectangle_value = load %Rectangle, ptr %Rectangle_obj, align 4
  store %Rectangle %Rectangle_value, ptr %r, align 4
  %p_value = load %Point, ptr %p, align 4
  %r_value = load %Rectangle, ptr %r, align 4
  %return_struct = alloca { %Point, %Rectangle }, align 8
  %field_ptr = getelementptr inbounds { %Point, %Rectangle }, ptr %return_struct, i32 0, i32 0
  store %Point %p_value, ptr %field_ptr, align 4
  %field_ptr1 = getelementptr inbounds { %Point, %Rectangle }, ptr %return_struct, i32 0, i32 1
  store %Rectangle %r_value, ptr %field_ptr1, align 4
  %return_value = load { %Point, %Rectangle }, ptr %return_struct, align 4
  ret { %Point, %Rectangle } %return_value
}

define { %Person, %Company } @createEntities() {
entry:
  %company = alloca %Company, align 8
  %person = alloca %Person, align 8
  %Person_obj = alloca %Person, align 8
  %id_ptr = getelementptr inbounds %Person, ptr %Person_obj, i32 0, i32 0
  store i64 42, ptr %id_ptr, align 4
  %name_ptr = getelementptr inbounds %Person, ptr %Person_obj, i32 0, i32 1
  store ptr @0, ptr %name_ptr, align 8
  %salary_ptr = getelementptr inbounds %Person, ptr %Person_obj, i32 0, i32 2
  store double 7.500050e+04, ptr %salary_ptr, align 8
  %Person_value = load %Person, ptr %Person_obj, align 8
  store %Person %Person_value, ptr %person, align 8
  %Company_obj = alloca %Company, align 8
  %name_ptr1 = getelementptr inbounds %Company, ptr %Company_obj, i32 0, i32 0
  store ptr @1, ptr %name_ptr1, align 8
  %employeeCount_ptr = getelementptr inbounds %Company, ptr %Company_obj, i32 0, i32 1
  store i64 250, ptr %employeeCount_ptr, align 4
  %isPublic_ptr = getelementptr inbounds %Company, ptr %Company_obj, i32 0, i32 2
  store i1 true, ptr %isPublic_ptr, align 1
  %Company_value = load %Company, ptr %Company_obj, align 8
  store %Company %Company_value, ptr %company, align 8
  %person_value = load %Person, ptr %person, align 8
  %company_value = load %Company, ptr %company, align 8
  %return_struct = alloca { %Person, %Company }, align 8
  %field_ptr = getelementptr inbounds { %Person, %Company }, ptr %return_struct, i32 0, i32 0
  store %Person %person_value, ptr %field_ptr, align 8
  %field_ptr2 = getelementptr inbounds { %Person, %Company }, ptr %return_struct, i32 0, i32 1
  store %Company %company_value, ptr %field_ptr2, align 8
  %return_value = load { %Person, %Company }, ptr %return_struct, align 8
  ret { %Person, %Company } %return_value
}

define i32 @main() {
entry:
  %company = alloca %Company, align 8
  %person = alloca %Person, align 8
  %r = alloca %Rectangle, align 8
  %p = alloca %Point, align 8
  %call_createShapes = call { %Point, %Rectangle } @createShapes()
  %p_value = extractvalue { %Point, %Rectangle } %call_createShapes, 0
  store %Point %p_value, ptr %p, align 4
  %r_value = extractvalue { %Point, %Rectangle } %call_createShapes, 1
  store %Rectangle %r_value, ptr %r, align 4
  %x_ptr = getelementptr inbounds %Point, ptr %p, i32 0, i32 0
  %p_value1 = load %Point, ptr %p, align 4
  %x_ptr2 = getelementptr inbounds %Point, ptr %p, i32 0, i32 0
  %x_load = load i64, ptr %x_ptr2, align 4
  %multmp = mul i64 %x_load, 2
  store i64 %multmp, ptr %x_ptr, align 4
  %height_ptr = getelementptr inbounds %Rectangle, ptr %r, i32 0, i32 1
  %r_value3 = load %Rectangle, ptr %r, align 4
  %height_ptr4 = getelementptr inbounds %Rectangle, ptr %r, i32 0, i32 1
  %height_load = load i64, ptr %height_ptr4, align 4
  %addtmp = add i64 %height_load, 25
  store i64 %addtmp, ptr %height_ptr, align 4
  %call_createEntities = call { %Person, %Company } @createEntities()
  %person_value = extractvalue { %Person, %Company } %call_createEntities, 0
  store %Person %person_value, ptr %person, align 8
  %company_value = extractvalue { %Person, %Company } %call_createEntities, 1
  store %Company %company_value, ptr %company, align 8
  %salary_ptr = getelementptr inbounds %Person, ptr %person, i32 0, i32 2
  %person_value5 = load %Person, ptr %person, align 8
  %salary_ptr6 = getelementptr inbounds %Person, ptr %person, i32 0, i32 2
  %salary_load = load double, ptr %salary_ptr6, align 8
  %fmultmp = fmul double %salary_load, 1.100000e+00
  store double %fmultmp, ptr %salary_ptr, align 8
  %employeeCount_ptr = getelementptr inbounds %Company, ptr %company, i32 0, i32 1
  %company_value7 = load %Company, ptr %company, align 8
  %employeeCount_ptr8 = getelementptr inbounds %Company, ptr %company, i32 0, i32 1
  %employeeCount_load = load i64, ptr %employeeCount_ptr8, align 4
  %addtmp9 = add i64 %employeeCount_load, 50
  store i64 %addtmp9, ptr %employeeCount_ptr, align 4
  %person_value10 = load %Person, ptr %person, align 8
  %company_value11 = load %Company, ptr %company, align 8
  %multi_return_struct = alloca { %Person, %Company }, align 8
  %field_ptr = getelementptr inbounds { %Person, %Company }, ptr %multi_return_struct, i32 0, i32 0
  store %Person %person_value10, ptr %field_ptr, align 8
  %field_ptr12 = getelementptr inbounds { %Person, %Company }, ptr %multi_return_struct, i32 0, i32 1
  store %Company %company_value11, ptr %field_ptr12, align 8
  %multi_return_value = load { %Person, %Company }, ptr %multi_return_struct, align 8
  %struct_temp = alloca { %Person, %Company }, align 8
  store { %Person, %Company } %multi_return_value, ptr %struct_temp, align 8
  %field_names_array = alloca [2 x ptr], align 8
  %element_ptr = getelementptr [2 x ptr], ptr %field_names_array, i32 0, i32 0
  store ptr @field_name, ptr %element_ptr, align 8
  %element_ptr13 = getelementptr [2 x ptr], ptr %field_names_array, i32 0, i32 1
  store ptr @field_name.1, ptr %element_ptr13, align 8
  %field_names_ptr = getelementptr [2 x ptr], ptr %field_names_array, i32 0, i32 0
  %json_result = call ptr @__vyn_serialize_struct_with_names(ptr %struct_temp, ptr @typename, ptr %field_names_ptr, i32 2)
  call void @__vyn_println(ptr %json_result)
  ret i32 0
}

declare ptr @__vyn_serialize_struct_with_names(ptr, ptr, ptr, i32)

declare void @__vyn_println(ptr)
