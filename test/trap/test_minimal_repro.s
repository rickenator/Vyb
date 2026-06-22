	.text
	.file	"VybModule"
	.globl	divide                          # -- Begin function divide
	.p2align	4, 0x90
	.type	divide,@function
divide:                                 # @divide
.Lfunc_begin0:
	.file	1 "/home/rick/Projects/Vyb/test/trap" "test_minimal_repro.vyb.ll"
	.loc	1 2 0                           # test_minimal_repro.vyb.ll:2:0
	.cfi_startproc
# %bb.0:                                # %entry
	subq	$24, %rsp
	.cfi_def_cfa_offset 32
.Ltmp0:
	.loc	1 2 1 prologue_end              # test_minimal_repro.vyb.ll:2:1
	movq	%rdi, 8(%rsp)
	movq	%rsi, 16(%rsp)
	testq	%rsi, %rsi
	je	.LBB0_1
# %bb.2:                                # %ifcont
	movq	8(%rsp), %rax
	cqto
	idivq	16(%rsp)
	xorl	%edx, %edx
	.loc	1 2 1 epilogue_begin is_stmt 0  # test_minimal_repro.vyb.ll:2:1
	addq	$24, %rsp
	.cfi_def_cfa_offset 8
	retq
.LBB0_1:                                # %then
	.cfi_def_cfa_offset 32
	movl	$16, %edi
	callq	malloc@PLT
	movq	$42, 8(%rax)
	movq	%rax, %rdx
	.loc	1 2 1 epilogue_begin            # test_minimal_repro.vyb.ll:2:1
	addq	$24, %rsp
	.cfi_def_cfa_offset 8
	retq
.Ltmp1:
.Lfunc_end0:
	.size	divide, .Lfunc_end0-divide
	.cfi_endproc
                                        # -- End function
	.globl	compute                         # -- Begin function compute
	.p2align	4, 0x90
	.type	compute,@function
compute:                                # @compute
.Lfunc_begin1:
	.loc	1 7 0 is_stmt 1                 # test_minimal_repro.vyb.ll:7:0
	.cfi_startproc
# %bb.0:                                # %entry
	subq	$24, %rsp
	.cfi_def_cfa_offset 32
.Ltmp2:
	.loc	1 7 1 prologue_end              # test_minimal_repro.vyb.ll:7:1
	movq	%rdi, (%rsp)
	movq	%rsi, 8(%rsp)
	callq	divide@PLT
	testq	%rdx, %rdx
	je	.LBB1_2
# %bb.1:                                # %call.error5
	.loc	1 7 1 epilogue_begin is_stmt 0  # test_minimal_repro.vyb.ll:7:1
	addq	$24, %rsp
	.cfi_def_cfa_offset 8
	retq
.LBB1_2:                                # %call.success
	.cfi_def_cfa_offset 32
	movq	%rax, 16(%rsp)
	addq	$10, %rax
	xorl	%edx, %edx
	.loc	1 7 1 epilogue_begin            # test_minimal_repro.vyb.ll:7:1
	addq	$24, %rsp
	.cfi_def_cfa_offset 8
	retq
.Ltmp3:
.Lfunc_end1:
	.size	compute, .Lfunc_end1-compute
	.cfi_endproc
                                        # -- End function
	.globl	main                            # -- Begin function main
	.p2align	4, 0x90
	.type	main,@function
main:                                   # @main
.Lfunc_begin2:
	.loc	1 12 0 is_stmt 1                # test_minimal_repro.vyb.ll:12:0
	.cfi_startproc
# %bb.0:                                # %entry
	subq	$24, %rsp
	.cfi_def_cfa_offset 32
.Ltmp4:
	.loc	1 12 1 prologue_end             # test_minimal_repro.vyb.ll:12:1
	movl	$10, %edi
	movl	$2, %esi
	callq	compute@PLT
	testq	%rdx, %rdx
	jne	.LBB2_6
# %bb.1:                                # %call.success
	movq	%rax, (%rsp)
	movl	$10, %edi
	xorl	%esi, %esi
	callq	compute@PLT
	testq	%rdx, %rdx
	je	.LBB2_4
# %bb.2:                                # %call.error6
	movq	%rdx, 8(%rsp)
	movabsq	$-3994496327427856726, %rax     # imm = 0xC890B2C1030F66AA
	cmpq	%rax, (%rdx)
	jne	.LBB2_5
# %bb.3:                                # %trap.handler0
	movq	%rdx, %rdi
	callq	free@PLT
	movq	$-1, %rax
.LBB2_4:                                # %block.continue
	movq	%rax, 16(%rsp)
	addq	(%rsp), %rax
	.loc	1 12 1 epilogue_begin is_stmt 0 # test_minimal_repro.vyb.ll:12:1
	addq	$24, %rsp
	.cfi_def_cfa_offset 8
	retq
.LBB2_6:                                # %call.error1
	.cfi_def_cfa_offset 32
	xorl	%edi, %edi
	callq	__vyb_runtime_untrapped_error@PLT
.LBB2_5:                                # %trap.unmatched
	movq	%rdx, %rdi
	callq	__vyb_runtime_untrapped_error@PLT
.Ltmp5:
.Lfunc_end2:
	.size	main, .Lfunc_end2-main
	.cfi_endproc
                                        # -- End function
	.section	.debug_abbrev,"",@progbits
	.byte	1                               # Abbreviation Code
	.byte	17                              # DW_TAG_compile_unit
	.byte	1                               # DW_CHILDREN_yes
	.byte	37                              # DW_AT_producer
	.byte	14                              # DW_FORM_strp
	.byte	19                              # DW_AT_language
	.byte	5                               # DW_FORM_data2
	.byte	3                               # DW_AT_name
	.byte	14                              # DW_FORM_strp
	.byte	16                              # DW_AT_stmt_list
	.byte	23                              # DW_FORM_sec_offset
	.byte	27                              # DW_AT_comp_dir
	.byte	14                              # DW_FORM_strp
	.ascii	"\264B"                         # DW_AT_GNU_pubnames
	.byte	25                              # DW_FORM_flag_present
	.byte	17                              # DW_AT_low_pc
	.byte	1                               # DW_FORM_addr
	.byte	18                              # DW_AT_high_pc
	.byte	6                               # DW_FORM_data4
	.byte	0                               # EOM(1)
	.byte	0                               # EOM(2)
	.byte	2                               # Abbreviation Code
	.byte	46                              # DW_TAG_subprogram
	.byte	1                               # DW_CHILDREN_yes
	.byte	17                              # DW_AT_low_pc
	.byte	1                               # DW_FORM_addr
	.byte	18                              # DW_AT_high_pc
	.byte	6                               # DW_FORM_data4
	.byte	64                              # DW_AT_frame_base
	.byte	24                              # DW_FORM_exprloc
	.byte	110                             # DW_AT_linkage_name
	.byte	14                              # DW_FORM_strp
	.byte	3                               # DW_AT_name
	.byte	14                              # DW_FORM_strp
	.byte	58                              # DW_AT_decl_file
	.byte	11                              # DW_FORM_data1
	.byte	59                              # DW_AT_decl_line
	.byte	11                              # DW_FORM_data1
	.byte	73                              # DW_AT_type
	.byte	19                              # DW_FORM_ref4
	.byte	63                              # DW_AT_external
	.byte	25                              # DW_FORM_flag_present
	.byte	0                               # EOM(1)
	.byte	0                               # EOM(2)
	.byte	3                               # Abbreviation Code
	.byte	52                              # DW_TAG_variable
	.byte	0                               # DW_CHILDREN_no
	.byte	2                               # DW_AT_location
	.byte	24                              # DW_FORM_exprloc
	.byte	3                               # DW_AT_name
	.byte	14                              # DW_FORM_strp
	.byte	58                              # DW_AT_decl_file
	.byte	11                              # DW_FORM_data1
	.byte	59                              # DW_AT_decl_line
	.byte	11                              # DW_FORM_data1
	.byte	73                              # DW_AT_type
	.byte	19                              # DW_FORM_ref4
	.byte	0                               # EOM(1)
	.byte	0                               # EOM(2)
	.byte	4                               # Abbreviation Code
	.byte	19                              # DW_TAG_structure_type
	.byte	0                               # DW_CHILDREN_no
	.byte	3                               # DW_AT_name
	.byte	14                              # DW_FORM_strp
	.byte	11                              # DW_AT_byte_size
	.byte	11                              # DW_FORM_data1
	.ascii	"\210\001"                      # DW_AT_alignment
	.byte	15                              # DW_FORM_udata
	.byte	0                               # EOM(1)
	.byte	0                               # EOM(2)
	.byte	5                               # Abbreviation Code
	.byte	36                              # DW_TAG_base_type
	.byte	0                               # DW_CHILDREN_no
	.byte	3                               # DW_AT_name
	.byte	14                              # DW_FORM_strp
	.byte	62                              # DW_AT_encoding
	.byte	11                              # DW_FORM_data1
	.byte	11                              # DW_AT_byte_size
	.byte	11                              # DW_FORM_data1
	.byte	0                               # EOM(1)
	.byte	0                               # EOM(2)
	.byte	0                               # EOM(3)
	.section	.debug_info,"",@progbits
.Lcu_begin0:
	.long	.Ldebug_info_end0-.Ldebug_info_start0 # Length of Unit
.Ldebug_info_start0:
	.short	4                               # DWARF version number
	.long	.debug_abbrev                   # Offset Into Abbrev. Section
	.byte	8                               # Address Size (in bytes)
	.byte	1                               # Abbrev [1] 0xb:0xea DW_TAG_compile_unit
	.long	.Linfo_string0                  # DW_AT_producer
	.short	4                               # DW_AT_language
	.long	.Linfo_string1                  # DW_AT_name
	.long	.Lline_table_start0             # DW_AT_stmt_list
	.long	.Linfo_string2                  # DW_AT_comp_dir
                                        # DW_AT_GNU_pubnames
	.quad	.Lfunc_begin0                   # DW_AT_low_pc
	.long	.Lfunc_end2-.Lfunc_begin0       # DW_AT_high_pc
	.byte	2                               # Abbrev [2] 0x2a:0x3a DW_TAG_subprogram
	.quad	.Lfunc_begin0                   # DW_AT_low_pc
	.long	.Lfunc_end0-.Lfunc_begin0       # DW_AT_high_pc
	.byte	1                               # DW_AT_frame_base
	.byte	87
	.long	.Linfo_string3                  # DW_AT_linkage_name
	.long	.Linfo_string3                  # DW_AT_name
	.byte	1                               # DW_AT_decl_file
	.byte	2                               # DW_AT_decl_line
	.long	230                             # DW_AT_type
                                        # DW_AT_external
	.byte	3                               # Abbrev [3] 0x47:0xe DW_TAG_variable
	.byte	2                               # DW_AT_location
	.byte	145
	.byte	8
	.long	.Linfo_string8                  # DW_AT_name
	.byte	1                               # DW_AT_decl_file
	.byte	2                               # DW_AT_decl_line
	.long	237                             # DW_AT_type
	.byte	3                               # Abbrev [3] 0x55:0xe DW_TAG_variable
	.byte	2                               # DW_AT_location
	.byte	145
	.byte	16
	.long	.Linfo_string9                  # DW_AT_name
	.byte	1                               # DW_AT_decl_file
	.byte	2                               # DW_AT_decl_line
	.long	237                             # DW_AT_type
	.byte	0                               # End Of Children Mark
	.byte	2                               # Abbrev [2] 0x64:0x48 DW_TAG_subprogram
	.quad	.Lfunc_begin1                   # DW_AT_low_pc
	.long	.Lfunc_end1-.Lfunc_begin1       # DW_AT_high_pc
	.byte	1                               # DW_AT_frame_base
	.byte	87
	.long	.Linfo_string5                  # DW_AT_linkage_name
	.long	.Linfo_string5                  # DW_AT_name
	.byte	1                               # DW_AT_decl_file
	.byte	7                               # DW_AT_decl_line
	.long	230                             # DW_AT_type
                                        # DW_AT_external
	.byte	3                               # Abbrev [3] 0x81:0xe DW_TAG_variable
	.byte	2                               # DW_AT_location
	.byte	145
	.byte	0
	.long	.Linfo_string10                 # DW_AT_name
	.byte	1                               # DW_AT_decl_file
	.byte	7                               # DW_AT_decl_line
	.long	237                             # DW_AT_type
	.byte	3                               # Abbrev [3] 0x8f:0xe DW_TAG_variable
	.byte	2                               # DW_AT_location
	.byte	145
	.byte	8
	.long	.Linfo_string11                 # DW_AT_name
	.byte	1                               # DW_AT_decl_file
	.byte	7                               # DW_AT_decl_line
	.long	237                             # DW_AT_type
	.byte	3                               # Abbrev [3] 0x9d:0xe DW_TAG_variable
	.byte	2                               # DW_AT_location
	.byte	145
	.byte	16
	.long	.Linfo_string12                 # DW_AT_name
	.byte	1                               # DW_AT_decl_file
	.byte	8                               # DW_AT_decl_line
	.long	237                             # DW_AT_type
	.byte	0                               # End Of Children Mark
	.byte	2                               # Abbrev [2] 0xac:0x3a DW_TAG_subprogram
	.quad	.Lfunc_begin2                   # DW_AT_low_pc
	.long	.Lfunc_end2-.Lfunc_begin2       # DW_AT_high_pc
	.byte	1                               # DW_AT_frame_base
	.byte	87
	.long	.Linfo_string6                  # DW_AT_linkage_name
	.long	.Linfo_string6                  # DW_AT_name
	.byte	1                               # DW_AT_decl_file
	.byte	12                              # DW_AT_decl_line
	.long	237                             # DW_AT_type
                                        # DW_AT_external
	.byte	3                               # Abbrev [3] 0xc9:0xe DW_TAG_variable
	.byte	2                               # DW_AT_location
	.byte	145
	.byte	0
	.long	.Linfo_string13                 # DW_AT_name
	.byte	1                               # DW_AT_decl_file
	.byte	14                              # DW_AT_decl_line
	.long	237                             # DW_AT_type
	.byte	3                               # Abbrev [3] 0xd7:0xe DW_TAG_variable
	.byte	2                               # DW_AT_location
	.byte	145
	.byte	16
	.long	.Linfo_string14                 # DW_AT_name
	.byte	1                               # DW_AT_decl_file
	.byte	17                              # DW_AT_decl_line
	.long	237                             # DW_AT_type
	.byte	0                               # End Of Children Mark
	.byte	4                               # Abbrev [4] 0xe6:0x7 DW_TAG_structure_type
	.long	.Linfo_string4                  # DW_AT_name
	.byte	16                              # DW_AT_byte_size
	.byte	1                               # DW_AT_alignment
	.byte	5                               # Abbrev [5] 0xed:0x7 DW_TAG_base_type
	.long	.Linfo_string7                  # DW_AT_name
	.byte	5                               # DW_AT_encoding
	.byte	8                               # DW_AT_byte_size
	.byte	0                               # End Of Children Mark
.Ldebug_info_end0:
	.section	.debug_str,"MS",@progbits,1
.Linfo_string0:
	.asciz	"Vyb Compiler"                  # string offset=0
.Linfo_string1:
	.asciz	"test_minimal_repro.vyb.ll"     # string offset=13
.Linfo_string2:
	.asciz	"/home/rick/Projects/Vyb/test/trap" # string offset=39
.Linfo_string3:
	.asciz	"divide"                        # string offset=73
.Linfo_string4:
	.asciz	"struct_return"                 # string offset=80
.Linfo_string5:
	.asciz	"compute"                       # string offset=94
.Linfo_string6:
	.asciz	"main"                          # string offset=102
.Linfo_string7:
	.asciz	"i64"                           # string offset=107
.Linfo_string8:
	.asciz	"a"                             # string offset=111
.Linfo_string9:
	.asciz	"b"                             # string offset=113
.Linfo_string10:
	.asciz	"x"                             # string offset=115
.Linfo_string11:
	.asciz	"y"                             # string offset=117
.Linfo_string12:
	.asciz	"result"                        # string offset=119
.Linfo_string13:
	.asciz	"val1"                          # string offset=126
.Linfo_string14:
	.asciz	"val2"                          # string offset=131
	.section	.debug_pubnames,"",@progbits
	.long	.LpubNames_end0-.LpubNames_start0 # Length of Public Names Info
.LpubNames_start0:
	.short	2                               # DWARF Version
	.long	.Lcu_begin0                     # Offset of Compilation Unit Info
	.long	245                             # Compilation Unit Length
	.long	42                              # DIE offset
	.asciz	"divide"                        # External Name
	.long	100                             # DIE offset
	.asciz	"compute"                       # External Name
	.long	172                             # DIE offset
	.asciz	"main"                          # External Name
	.long	0                               # End Mark
.LpubNames_end0:
	.section	.debug_pubtypes,"",@progbits
	.long	.LpubTypes_end0-.LpubTypes_start0 # Length of Public Types Info
.LpubTypes_start0:
	.short	2                               # DWARF Version
	.long	.Lcu_begin0                     # Offset of Compilation Unit Info
	.long	245                             # Compilation Unit Length
	.long	230                             # DIE offset
	.asciz	"struct_return"                 # External Name
	.long	237                             # DIE offset
	.asciz	"i64"                           # External Name
	.long	0                               # End Mark
.LpubTypes_end0:
	.section	".note.GNU-stack","",@progbits
	.section	.debug_line,"",@progbits
.Lline_table_start0:
