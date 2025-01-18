SECTION .symbols
global kernel_symbols_start
kernel_symbols_start:

    extern ActiveTasks
    align 8
    dq ActiveTasks
    dq 0x1000
    db 'ActiveTasks', 0

    extern CPULocals
    align 8
    dq CPULocals
    dq 0x4000
    db 'CPULocals', 0

    extern CpuSimdRegionSize
    align 8
    dq CpuSimdRegionSize
    dq 0x8
    db 'CpuSimdRegionSize', 0

    extern DefaultFxstate
    align 8
    dq DefaultFxstate
    dq 0x8
    db 'DefaultFxstate', 0

    extern DevFSList
    align 8
    dq DevFSList
    dq 0x8
    db 'DevFSList', 0

    extern IRQFunctions
    align 8
    dq IRQFunctions
    dq 0x200
    db 'IRQFunctions', 0

    extern KernelPml4
    align 8
    dq KernelPml4
    dq 0x8
    db 'KernelPml4', 0

    extern KernelSymbolsHashmap
    align 8
    dq KernelSymbolsHashmap
    dq 0x8
    db 'KernelSymbolsHashmap', 0

    extern NVMEDevices
    align 8
    dq NVMEDevices
    dq 0x8
    db 'NVMEDevices', 0

    extern ReSchedulerLock
    align 8
    dq ReSchedulerLock
    dq 0x20
    db 'ReSchedulerLock', 0

    extern SchedulerLock
    align 8
    dq SchedulerLock
    dq 0x20
    db 'SchedulerLock', 0

    extern SchedulerProcesses
    align 8
    dq SchedulerProcesses
    dq 0x800
    db 'SchedulerProcesses', 0

    extern SchedulerRunning
    align 8
    dq SchedulerRunning
    dq 0x1
    db 'SchedulerRunning', 0

    extern TYPE_CHECK_KINFS
    align 8
    dq TYPE_CHECK_KINFS
    dq 0x50
    db 'TYPE_CHECK_KINFS', 0

    extern TimerCounter
    align 8
    dq TimerCounter
    dq 0x8
    db 'TimerCounter', 0

    extern TimerSeconds
    align 8
    dq TimerSeconds
    dq 0x8
    db 'TimerSeconds', 0

    extern VFSTree
    align 8
    dq VFSTree
    dq 0x8
    db 'VFSTree', 0

    extern __ubsan_handle_add_overflow
    align 8
    dq __ubsan_handle_add_overflow
    dq 0x1f
    db '__ubsan_handle_add_overflow', 0

    extern __ubsan_handle_divrem_overflow
    align 8
    dq __ubsan_handle_divrem_overflow
    dq 0x1f
    db '__ubsan_handle_divrem_overflow', 0

    extern __ubsan_handle_function_type_mismatch
    align 8
    dq __ubsan_handle_function_type_mismatch
    dq 0x24
    db '__ubsan_handle_function_type_mismatch', 0

    extern __ubsan_handle_invalid_builtin
    align 8
    dq __ubsan_handle_invalid_builtin
    dq 0x24
    db '__ubsan_handle_invalid_builtin', 0

    extern __ubsan_handle_load_invalid_value
    align 8
    dq __ubsan_handle_load_invalid_value
    dq 0x3f
    db '__ubsan_handle_load_invalid_value', 0

    extern __ubsan_handle_mul_overflow
    align 8
    dq __ubsan_handle_mul_overflow
    dq 0x24
    db '__ubsan_handle_mul_overflow', 0

    extern __ubsan_handle_negate_overflow
    align 8
    dq __ubsan_handle_negate_overflow
    dq 0x24
    db '__ubsan_handle_negate_overflow', 0

    extern __ubsan_handle_out_of_bounds
    align 8
    dq __ubsan_handle_out_of_bounds
    dq 0xf1
    db '__ubsan_handle_out_of_bounds', 0

    extern __ubsan_handle_pointer_overflow
    align 8
    dq __ubsan_handle_pointer_overflow
    dq 0x1f
    db '__ubsan_handle_pointer_overflow', 0

    extern __ubsan_handle_shift_out_of_bounds
    align 8
    dq __ubsan_handle_shift_out_of_bounds
    dq 0x3a
    db '__ubsan_handle_shift_out_of_bounds', 0

    extern __ubsan_handle_sub_overflow
    align 8
    dq __ubsan_handle_sub_overflow
    dq 0xc2
    db '__ubsan_handle_sub_overflow', 0

    extern __ubsan_handle_type_mismatch
    align 8
    dq __ubsan_handle_type_mismatch
    dq 0x75
    db '__ubsan_handle_type_mismatch', 0

    extern __ubsan_handle_type_mismatch_v1
    align 8
    dq __ubsan_handle_type_mismatch_v1
    dq 0xa0
    db '__ubsan_handle_type_mismatch_v1', 0

    extern _vsnprintf_internal
    align 8
    dq _vsnprintf_internal
    dq 0x1424
    db '_vsnprintf_internal', 0

    extern acpi_get_table
    align 8
    dq acpi_get_table
    dq 0x25c
    db 'acpi_get_table', 0

    extern acpi_init
    align 8
    dq acpi_init
    dq 0x6ac
    db 'acpi_init', 0

    extern acpi_post_init
    align 8
    dq acpi_post_init
    dq 0x41
    db 'acpi_post_init', 0

    extern bswap16
    align 8
    dq bswap16
    dq 0x7
    db 'bswap16', 0

    extern bswap32
    align 8
    dq bswap32
    dq 0x5
    db 'bswap32', 0

    extern bswap64
    align 8
    dq bswap64
    dq 0x7
    db 'bswap64', 0

    extern char_to_hex
    align 8
    dq char_to_hex
    dq 0x25
    db 'char_to_hex', 0

    extern cpu_restore_simd
    align 8
    dq cpu_restore_simd
    dq 0x8
    db 'cpu_restore_simd', 0

    extern cpu_save_simd
    align 8
    dq cpu_save_simd
    dq 0x8
    db 'cpu_save_simd', 0

    extern crc32_calculate_buffer
    align 8
    dq crc32_calculate_buffer
    dq 0x108
    db 'crc32_calculate_buffer', 0

    extern devfs_finddir
    align 8
    dq devfs_finddir
    dq 0x7a6
    db 'devfs_finddir', 0

    extern devfs_init
    align 8
    dq devfs_init
    dq 0x2dc
    db 'devfs_init', 0

    extern devfs_mount
    align 8
    dq devfs_mount
    dq 0xc7
    db 'devfs_mount', 0

    extern devfs_print_tree
    align 8
    dq devfs_print_tree
    dq 0x3f9
    db 'devfs_print_tree', 0

    extern dispatch_interrupt
    align 8
    dq dispatch_interrupt
    dq 0xba0
    db 'dispatch_interrupt', 0

    extern elf_load
    align 8
    dq elf_load
    dq 0x525
    db 'elf_load', 0

    extern event_array_await_timeout
    align 8
    dq event_array_await_timeout
    dq 0x186
    db 'event_array_await_timeout', 0

    extern event_await
    align 8
    dq event_await
    dq 0x28
    db 'event_await', 0

    extern event_await_timeout
    align 8
    dq event_await_timeout
    dq 0x2b
    db 'event_await_timeout', 0

    extern event_notify
    align 8
    dq event_notify
    dq 0xa
    db 'event_notify', 0

    extern event_reset
    align 8
    dq event_reset
    dq 0x28
    db 'event_reset', 0

    extern events_await
    align 8
    dq events_await
    dq 0x3c5
    db 'events_await', 0

    extern exec
    align 8
    dq exec
    dq 0x54b
    db 'exec', 0

    extern fat32_finddir
    align 8
    dq fat32_finddir
    dq 0x1679
    db 'fat32_finddir', 0

    extern fat32_init
    align 8
    dq fat32_init
    dq 0x11e
    db 'fat32_init', 0

    extern fat32_lseek
    align 8
    dq fat32_lseek
    dq 0x73c
    db 'fat32_lseek', 0

    extern fat32_mount
    align 8
    dq fat32_mount
    dq 0x5c6
    db 'fat32_mount', 0

    extern fat32_print_tree
    align 8
    dq fat32_print_tree
    dq 0x431
    db 'fat32_print_tree', 0

    extern fat32_read
    align 8
    dq fat32_read
    dq 0x1087
    db 'fat32_read', 0

    extern fat32_readdir
    align 8
    dq fat32_readdir
    dq 0xf6c
    db 'fat32_readdir', 0

    extern force_reschedule
    align 8
    dq force_reschedule
    dq 0x0
    db 'force_reschedule', 0

    extern gdt_init
    align 8
    dq gdt_init
    dq 0x122
    db 'gdt_init', 0

    extern gdt_setup_tss
    align 8
    dq gdt_setup_tss
    dq 0x64
    db 'gdt_setup_tss', 0

    extern gentree_create
    align 8
    dq gentree_create
    dq 0xe2
    db 'gentree_create', 0

    extern gentree_get_parent
    align 8
    dq gentree_get_parent
    dq 0x1e3
    db 'gentree_get_parent', 0

    extern gentree_insert
    align 8
    dq gentree_insert
    dq 0x209
    db 'gentree_insert', 0

    extern gentree_node_create
    align 8
    dq gentree_node_create
    dq 0xb2
    db 'gentree_node_create', 0

    extern gentree_remove
    align 8
    dq gentree_remove
    dq 0x7f
    db 'gentree_remove', 0

    extern hashmap_create
    align 8
    dq hashmap_create
    dq 0x143
    db 'hashmap_create', 0

    extern hashmap_delete
    align 8
    dq hashmap_delete
    dq 0x29e
    db 'hashmap_delete', 0

    extern hashmap_get
    align 8
    dq hashmap_get
    dq 0x310
    db 'hashmap_get', 0

    extern hashmap_has
    align 8
    dq hashmap_has
    dq 0x308
    db 'hashmap_has', 0

    extern hashmap_remove
    align 8
    dq hashmap_remove
    dq 0x4a1
    db 'hashmap_remove', 0

    extern hashmap_set
    align 8
    dq hashmap_set
    dq 0x579
    db 'hashmap_set', 0

    extern hashmap_string_hash
    align 8
    dq hashmap_string_hash
    dq 0x90
    db 'hashmap_string_hash', 0

    extern hashmap_to_list
    align 8
    dq hashmap_to_list
    dq 0x2c1
    db 'hashmap_to_list', 0

    extern idt_add_pci_handler
    align 8
    dq idt_add_pci_handler
    dq 0x282
    db 'idt_add_pci_handler', 0

    extern idt_init
    align 8
    dq idt_init
    dq 0xe9
    db 'idt_init', 0

    extern ioapic_get_id_from_gsi
    align 8
    dq ioapic_get_id_from_gsi
    dq 0x1bd
    db 'ioapic_get_id_from_gsi', 0

    extern ioapic_read
    align 8
    dq ioapic_read
    dq 0x10a
    db 'ioapic_read', 0

    extern ioapic_redirect_gsi
    align 8
    dq ioapic_redirect_gsi
    dq 0x1d0
    db 'ioapic_redirect_gsi', 0

    extern ioapic_redirect_legacy_irq
    align 8
    dq ioapic_redirect_legacy_irq
    dq 0x213
    db 'ioapic_redirect_legacy_irq', 0

    extern ioapic_write
    align 8
    dq ioapic_write
    dq 0x135
    db 'ioapic_write', 0

    extern isr0
    align 8
    dq isr0
    dq 0x0
    db 'isr0', 0

    extern isr1
    align 8
    dq isr1
    dq 0x0
    db 'isr1', 0

    extern isr10
    align 8
    dq isr10
    dq 0x0
    db 'isr10', 0

    extern isr100
    align 8
    dq isr100
    dq 0x0
    db 'isr100', 0

    extern isr101
    align 8
    dq isr101
    dq 0x0
    db 'isr101', 0

    extern isr102
    align 8
    dq isr102
    dq 0x0
    db 'isr102', 0

    extern isr103
    align 8
    dq isr103
    dq 0x0
    db 'isr103', 0

    extern isr104
    align 8
    dq isr104
    dq 0x0
    db 'isr104', 0

    extern isr105
    align 8
    dq isr105
    dq 0x0
    db 'isr105', 0

    extern isr106
    align 8
    dq isr106
    dq 0x0
    db 'isr106', 0

    extern isr107
    align 8
    dq isr107
    dq 0x0
    db 'isr107', 0

    extern isr108
    align 8
    dq isr108
    dq 0x0
    db 'isr108', 0

    extern isr109
    align 8
    dq isr109
    dq 0x0
    db 'isr109', 0

    extern isr11
    align 8
    dq isr11
    dq 0x0
    db 'isr11', 0

    extern isr110
    align 8
    dq isr110
    dq 0x0
    db 'isr110', 0

    extern isr111
    align 8
    dq isr111
    dq 0x0
    db 'isr111', 0

    extern isr112
    align 8
    dq isr112
    dq 0x0
    db 'isr112', 0

    extern isr113
    align 8
    dq isr113
    dq 0x0
    db 'isr113', 0

    extern isr114
    align 8
    dq isr114
    dq 0x0
    db 'isr114', 0

    extern isr115
    align 8
    dq isr115
    dq 0x0
    db 'isr115', 0

    extern isr116
    align 8
    dq isr116
    dq 0x0
    db 'isr116', 0

    extern isr117
    align 8
    dq isr117
    dq 0x0
    db 'isr117', 0

    extern isr118
    align 8
    dq isr118
    dq 0x0
    db 'isr118', 0

    extern isr119
    align 8
    dq isr119
    dq 0x0
    db 'isr119', 0

    extern isr12
    align 8
    dq isr12
    dq 0x0
    db 'isr12', 0

    extern isr120
    align 8
    dq isr120
    dq 0x0
    db 'isr120', 0

    extern isr121
    align 8
    dq isr121
    dq 0x0
    db 'isr121', 0

    extern isr122
    align 8
    dq isr122
    dq 0x0
    db 'isr122', 0

    extern isr123
    align 8
    dq isr123
    dq 0x0
    db 'isr123', 0

    extern isr124
    align 8
    dq isr124
    dq 0x0
    db 'isr124', 0

    extern isr125
    align 8
    dq isr125
    dq 0x0
    db 'isr125', 0

    extern isr126
    align 8
    dq isr126
    dq 0x0
    db 'isr126', 0

    extern isr127
    align 8
    dq isr127
    dq 0x0
    db 'isr127', 0

    extern isr128
    align 8
    dq isr128
    dq 0x0
    db 'isr128', 0

    extern isr129
    align 8
    dq isr129
    dq 0x0
    db 'isr129', 0

    extern isr13
    align 8
    dq isr13
    dq 0x0
    db 'isr13', 0

    extern isr130
    align 8
    dq isr130
    dq 0x0
    db 'isr130', 0

    extern isr131
    align 8
    dq isr131
    dq 0x0
    db 'isr131', 0

    extern isr132
    align 8
    dq isr132
    dq 0x0
    db 'isr132', 0

    extern isr133
    align 8
    dq isr133
    dq 0x0
    db 'isr133', 0

    extern isr134
    align 8
    dq isr134
    dq 0x0
    db 'isr134', 0

    extern isr135
    align 8
    dq isr135
    dq 0x0
    db 'isr135', 0

    extern isr136
    align 8
    dq isr136
    dq 0x0
    db 'isr136', 0

    extern isr137
    align 8
    dq isr137
    dq 0x0
    db 'isr137', 0

    extern isr138
    align 8
    dq isr138
    dq 0x0
    db 'isr138', 0

    extern isr139
    align 8
    dq isr139
    dq 0x0
    db 'isr139', 0

    extern isr14
    align 8
    dq isr14
    dq 0x0
    db 'isr14', 0

    extern isr140
    align 8
    dq isr140
    dq 0x0
    db 'isr140', 0

    extern isr141
    align 8
    dq isr141
    dq 0x0
    db 'isr141', 0

    extern isr142
    align 8
    dq isr142
    dq 0x0
    db 'isr142', 0

    extern isr143
    align 8
    dq isr143
    dq 0x0
    db 'isr143', 0

    extern isr144
    align 8
    dq isr144
    dq 0x0
    db 'isr144', 0

    extern isr145
    align 8
    dq isr145
    dq 0x0
    db 'isr145', 0

    extern isr146
    align 8
    dq isr146
    dq 0x0
    db 'isr146', 0

    extern isr147
    align 8
    dq isr147
    dq 0x0
    db 'isr147', 0

    extern isr148
    align 8
    dq isr148
    dq 0x0
    db 'isr148', 0

    extern isr149
    align 8
    dq isr149
    dq 0x0
    db 'isr149', 0

    extern isr15
    align 8
    dq isr15
    dq 0x0
    db 'isr15', 0

    extern isr150
    align 8
    dq isr150
    dq 0x0
    db 'isr150', 0

    extern isr151
    align 8
    dq isr151
    dq 0x0
    db 'isr151', 0

    extern isr152
    align 8
    dq isr152
    dq 0x0
    db 'isr152', 0

    extern isr153
    align 8
    dq isr153
    dq 0x0
    db 'isr153', 0

    extern isr154
    align 8
    dq isr154
    dq 0x0
    db 'isr154', 0

    extern isr155
    align 8
    dq isr155
    dq 0x0
    db 'isr155', 0

    extern isr156
    align 8
    dq isr156
    dq 0x0
    db 'isr156', 0

    extern isr157
    align 8
    dq isr157
    dq 0x0
    db 'isr157', 0

    extern isr158
    align 8
    dq isr158
    dq 0x0
    db 'isr158', 0

    extern isr159
    align 8
    dq isr159
    dq 0x0
    db 'isr159', 0

    extern isr16
    align 8
    dq isr16
    dq 0x0
    db 'isr16', 0

    extern isr160
    align 8
    dq isr160
    dq 0x0
    db 'isr160', 0

    extern isr161
    align 8
    dq isr161
    dq 0x0
    db 'isr161', 0

    extern isr162
    align 8
    dq isr162
    dq 0x0
    db 'isr162', 0

    extern isr163
    align 8
    dq isr163
    dq 0x0
    db 'isr163', 0

    extern isr164
    align 8
    dq isr164
    dq 0x0
    db 'isr164', 0

    extern isr165
    align 8
    dq isr165
    dq 0x0
    db 'isr165', 0

    extern isr166
    align 8
    dq isr166
    dq 0x0
    db 'isr166', 0

    extern isr167
    align 8
    dq isr167
    dq 0x0
    db 'isr167', 0

    extern isr168
    align 8
    dq isr168
    dq 0x0
    db 'isr168', 0

    extern isr169
    align 8
    dq isr169
    dq 0x0
    db 'isr169', 0

    extern isr17
    align 8
    dq isr17
    dq 0x0
    db 'isr17', 0

    extern isr170
    align 8
    dq isr170
    dq 0x0
    db 'isr170', 0

    extern isr171
    align 8
    dq isr171
    dq 0x0
    db 'isr171', 0

    extern isr172
    align 8
    dq isr172
    dq 0x0
    db 'isr172', 0

    extern isr173
    align 8
    dq isr173
    dq 0x0
    db 'isr173', 0

    extern isr174
    align 8
    dq isr174
    dq 0x0
    db 'isr174', 0

    extern isr175
    align 8
    dq isr175
    dq 0x0
    db 'isr175', 0

    extern isr176
    align 8
    dq isr176
    dq 0x0
    db 'isr176', 0

    extern isr177
    align 8
    dq isr177
    dq 0x0
    db 'isr177', 0

    extern isr178
    align 8
    dq isr178
    dq 0x0
    db 'isr178', 0

    extern isr179
    align 8
    dq isr179
    dq 0x0
    db 'isr179', 0

    extern isr18
    align 8
    dq isr18
    dq 0x0
    db 'isr18', 0

    extern isr180
    align 8
    dq isr180
    dq 0x0
    db 'isr180', 0

    extern isr181
    align 8
    dq isr181
    dq 0x0
    db 'isr181', 0

    extern isr182
    align 8
    dq isr182
    dq 0x0
    db 'isr182', 0

    extern isr183
    align 8
    dq isr183
    dq 0x0
    db 'isr183', 0

    extern isr184
    align 8
    dq isr184
    dq 0x0
    db 'isr184', 0

    extern isr185
    align 8
    dq isr185
    dq 0x0
    db 'isr185', 0

    extern isr186
    align 8
    dq isr186
    dq 0x0
    db 'isr186', 0

    extern isr187
    align 8
    dq isr187
    dq 0x0
    db 'isr187', 0

    extern isr188
    align 8
    dq isr188
    dq 0x0
    db 'isr188', 0

    extern isr189
    align 8
    dq isr189
    dq 0x0
    db 'isr189', 0

    extern isr19
    align 8
    dq isr19
    dq 0x0
    db 'isr19', 0

    extern isr190
    align 8
    dq isr190
    dq 0x0
    db 'isr190', 0

    extern isr191
    align 8
    dq isr191
    dq 0x0
    db 'isr191', 0

    extern isr192
    align 8
    dq isr192
    dq 0x0
    db 'isr192', 0

    extern isr193
    align 8
    dq isr193
    dq 0x0
    db 'isr193', 0

    extern isr194
    align 8
    dq isr194
    dq 0x0
    db 'isr194', 0

    extern isr195
    align 8
    dq isr195
    dq 0x0
    db 'isr195', 0

    extern isr196
    align 8
    dq isr196
    dq 0x0
    db 'isr196', 0

    extern isr197
    align 8
    dq isr197
    dq 0x0
    db 'isr197', 0

    extern isr198
    align 8
    dq isr198
    dq 0x0
    db 'isr198', 0

    extern isr199
    align 8
    dq isr199
    dq 0x0
    db 'isr199', 0

    extern isr2
    align 8
    dq isr2
    dq 0x0
    db 'isr2', 0

    extern isr20
    align 8
    dq isr20
    dq 0x0
    db 'isr20', 0

    extern isr200
    align 8
    dq isr200
    dq 0x0
    db 'isr200', 0

    extern isr201
    align 8
    dq isr201
    dq 0x0
    db 'isr201', 0

    extern isr202
    align 8
    dq isr202
    dq 0x0
    db 'isr202', 0

    extern isr203
    align 8
    dq isr203
    dq 0x0
    db 'isr203', 0

    extern isr204
    align 8
    dq isr204
    dq 0x0
    db 'isr204', 0

    extern isr205
    align 8
    dq isr205
    dq 0x0
    db 'isr205', 0

    extern isr206
    align 8
    dq isr206
    dq 0x0
    db 'isr206', 0

    extern isr207
    align 8
    dq isr207
    dq 0x0
    db 'isr207', 0

    extern isr208
    align 8
    dq isr208
    dq 0x0
    db 'isr208', 0

    extern isr209
    align 8
    dq isr209
    dq 0x0
    db 'isr209', 0

    extern isr21
    align 8
    dq isr21
    dq 0x0
    db 'isr21', 0

    extern isr210
    align 8
    dq isr210
    dq 0x0
    db 'isr210', 0

    extern isr211
    align 8
    dq isr211
    dq 0x0
    db 'isr211', 0

    extern isr212
    align 8
    dq isr212
    dq 0x0
    db 'isr212', 0

    extern isr213
    align 8
    dq isr213
    dq 0x0
    db 'isr213', 0

    extern isr214
    align 8
    dq isr214
    dq 0x0
    db 'isr214', 0

    extern isr215
    align 8
    dq isr215
    dq 0x0
    db 'isr215', 0

    extern isr216
    align 8
    dq isr216
    dq 0x0
    db 'isr216', 0

    extern isr217
    align 8
    dq isr217
    dq 0x0
    db 'isr217', 0

    extern isr218
    align 8
    dq isr218
    dq 0x0
    db 'isr218', 0

    extern isr219
    align 8
    dq isr219
    dq 0x0
    db 'isr219', 0

    extern isr22
    align 8
    dq isr22
    dq 0x0
    db 'isr22', 0

    extern isr220
    align 8
    dq isr220
    dq 0x0
    db 'isr220', 0

    extern isr221
    align 8
    dq isr221
    dq 0x0
    db 'isr221', 0

    extern isr222
    align 8
    dq isr222
    dq 0x0
    db 'isr222', 0

    extern isr223
    align 8
    dq isr223
    dq 0x0
    db 'isr223', 0

    extern isr224
    align 8
    dq isr224
    dq 0x0
    db 'isr224', 0

    extern isr225
    align 8
    dq isr225
    dq 0x0
    db 'isr225', 0

    extern isr226
    align 8
    dq isr226
    dq 0x0
    db 'isr226', 0

    extern isr227
    align 8
    dq isr227
    dq 0x0
    db 'isr227', 0

    extern isr228
    align 8
    dq isr228
    dq 0x0
    db 'isr228', 0

    extern isr229
    align 8
    dq isr229
    dq 0x0
    db 'isr229', 0

    extern isr23
    align 8
    dq isr23
    dq 0x0
    db 'isr23', 0

    extern isr230
    align 8
    dq isr230
    dq 0x0
    db 'isr230', 0

    extern isr231
    align 8
    dq isr231
    dq 0x0
    db 'isr231', 0

    extern isr232
    align 8
    dq isr232
    dq 0x0
    db 'isr232', 0

    extern isr233
    align 8
    dq isr233
    dq 0x0
    db 'isr233', 0

    extern isr234
    align 8
    dq isr234
    dq 0x0
    db 'isr234', 0

    extern isr235
    align 8
    dq isr235
    dq 0x0
    db 'isr235', 0

    extern isr236
    align 8
    dq isr236
    dq 0x0
    db 'isr236', 0

    extern isr237
    align 8
    dq isr237
    dq 0x0
    db 'isr237', 0

    extern isr238
    align 8
    dq isr238
    dq 0x0
    db 'isr238', 0

    extern isr239
    align 8
    dq isr239
    dq 0x0
    db 'isr239', 0

    extern isr24
    align 8
    dq isr24
    dq 0x0
    db 'isr24', 0

    extern isr240
    align 8
    dq isr240
    dq 0x0
    db 'isr240', 0

    extern isr241
    align 8
    dq isr241
    dq 0x0
    db 'isr241', 0

    extern isr242
    align 8
    dq isr242
    dq 0x0
    db 'isr242', 0

    extern isr243
    align 8
    dq isr243
    dq 0x0
    db 'isr243', 0

    extern isr244
    align 8
    dq isr244
    dq 0x0
    db 'isr244', 0

    extern isr245
    align 8
    dq isr245
    dq 0x0
    db 'isr245', 0

    extern isr246
    align 8
    dq isr246
    dq 0x0
    db 'isr246', 0

    extern isr247
    align 8
    dq isr247
    dq 0x0
    db 'isr247', 0

    extern isr248
    align 8
    dq isr248
    dq 0x0
    db 'isr248', 0

    extern isr249
    align 8
    dq isr249
    dq 0x0
    db 'isr249', 0

    extern isr25
    align 8
    dq isr25
    dq 0x0
    db 'isr25', 0

    extern isr250
    align 8
    dq isr250
    dq 0x0
    db 'isr250', 0

    extern isr251
    align 8
    dq isr251
    dq 0x0
    db 'isr251', 0

    extern isr252
    align 8
    dq isr252
    dq 0x0
    db 'isr252', 0

    extern isr253
    align 8
    dq isr253
    dq 0x0
    db 'isr253', 0

    extern isr254
    align 8
    dq isr254
    dq 0x0
    db 'isr254', 0

    extern isr255
    align 8
    dq isr255
    dq 0x0
    db 'isr255', 0

    extern isr26
    align 8
    dq isr26
    dq 0x0
    db 'isr26', 0

    extern isr27
    align 8
    dq isr27
    dq 0x0
    db 'isr27', 0

    extern isr28
    align 8
    dq isr28
    dq 0x0
    db 'isr28', 0

    extern isr29
    align 8
    dq isr29
    dq 0x0
    db 'isr29', 0

    extern isr3
    align 8
    dq isr3
    dq 0x0
    db 'isr3', 0

    extern isr30
    align 8
    dq isr30
    dq 0x0
    db 'isr30', 0

    extern isr31
    align 8
    dq isr31
    dq 0x0
    db 'isr31', 0

    extern isr32
    align 8
    dq isr32
    dq 0x0
    db 'isr32', 0

    extern isr33
    align 8
    dq isr33
    dq 0x0
    db 'isr33', 0

    extern isr34
    align 8
    dq isr34
    dq 0x0
    db 'isr34', 0

    extern isr35
    align 8
    dq isr35
    dq 0x0
    db 'isr35', 0

    extern isr36
    align 8
    dq isr36
    dq 0x0
    db 'isr36', 0

    extern isr37
    align 8
    dq isr37
    dq 0x0
    db 'isr37', 0

    extern isr38
    align 8
    dq isr38
    dq 0x0
    db 'isr38', 0

    extern isr39
    align 8
    dq isr39
    dq 0x0
    db 'isr39', 0

    extern isr4
    align 8
    dq isr4
    dq 0x0
    db 'isr4', 0

    extern isr40
    align 8
    dq isr40
    dq 0x0
    db 'isr40', 0

    extern isr41
    align 8
    dq isr41
    dq 0x0
    db 'isr41', 0

    extern isr42
    align 8
    dq isr42
    dq 0x0
    db 'isr42', 0

    extern isr43
    align 8
    dq isr43
    dq 0x0
    db 'isr43', 0

    extern isr44
    align 8
    dq isr44
    dq 0x0
    db 'isr44', 0

    extern isr45
    align 8
    dq isr45
    dq 0x0
    db 'isr45', 0

    extern isr46
    align 8
    dq isr46
    dq 0x0
    db 'isr46', 0

    extern isr47
    align 8
    dq isr47
    dq 0x0
    db 'isr47', 0

    extern isr48
    align 8
    dq isr48
    dq 0x0
    db 'isr48', 0

    extern isr49
    align 8
    dq isr49
    dq 0x0
    db 'isr49', 0

    extern isr5
    align 8
    dq isr5
    dq 0x0
    db 'isr5', 0

    extern isr50
    align 8
    dq isr50
    dq 0x0
    db 'isr50', 0

    extern isr51
    align 8
    dq isr51
    dq 0x0
    db 'isr51', 0

    extern isr52
    align 8
    dq isr52
    dq 0x0
    db 'isr52', 0

    extern isr53
    align 8
    dq isr53
    dq 0x0
    db 'isr53', 0

    extern isr54
    align 8
    dq isr54
    dq 0x0
    db 'isr54', 0

    extern isr55
    align 8
    dq isr55
    dq 0x0
    db 'isr55', 0

    extern isr56
    align 8
    dq isr56
    dq 0x0
    db 'isr56', 0

    extern isr57
    align 8
    dq isr57
    dq 0x0
    db 'isr57', 0

    extern isr58
    align 8
    dq isr58
    dq 0x0
    db 'isr58', 0

    extern isr59
    align 8
    dq isr59
    dq 0x0
    db 'isr59', 0

    extern isr6
    align 8
    dq isr6
    dq 0x0
    db 'isr6', 0

    extern isr60
    align 8
    dq isr60
    dq 0x0
    db 'isr60', 0

    extern isr61
    align 8
    dq isr61
    dq 0x0
    db 'isr61', 0

    extern isr62
    align 8
    dq isr62
    dq 0x0
    db 'isr62', 0

    extern isr63
    align 8
    dq isr63
    dq 0x0
    db 'isr63', 0

    extern isr64
    align 8
    dq isr64
    dq 0x0
    db 'isr64', 0

    extern isr65
    align 8
    dq isr65
    dq 0x0
    db 'isr65', 0

    extern isr66
    align 8
    dq isr66
    dq 0x0
    db 'isr66', 0

    extern isr67
    align 8
    dq isr67
    dq 0x0
    db 'isr67', 0

    extern isr68
    align 8
    dq isr68
    dq 0x0
    db 'isr68', 0

    extern isr69
    align 8
    dq isr69
    dq 0x0
    db 'isr69', 0

    extern isr7
    align 8
    dq isr7
    dq 0x0
    db 'isr7', 0

    extern isr70
    align 8
    dq isr70
    dq 0x0
    db 'isr70', 0

    extern isr71
    align 8
    dq isr71
    dq 0x0
    db 'isr71', 0

    extern isr72
    align 8
    dq isr72
    dq 0x0
    db 'isr72', 0

    extern isr73
    align 8
    dq isr73
    dq 0x0
    db 'isr73', 0

    extern isr74
    align 8
    dq isr74
    dq 0x0
    db 'isr74', 0

    extern isr75
    align 8
    dq isr75
    dq 0x0
    db 'isr75', 0

    extern isr76
    align 8
    dq isr76
    dq 0x0
    db 'isr76', 0

    extern isr77
    align 8
    dq isr77
    dq 0x0
    db 'isr77', 0

    extern isr78
    align 8
    dq isr78
    dq 0x0
    db 'isr78', 0

    extern isr79
    align 8
    dq isr79
    dq 0x0
    db 'isr79', 0

    extern isr8
    align 8
    dq isr8
    dq 0x0
    db 'isr8', 0

    extern isr80
    align 8
    dq isr80
    dq 0x0
    db 'isr80', 0

    extern isr81
    align 8
    dq isr81
    dq 0x0
    db 'isr81', 0

    extern isr82
    align 8
    dq isr82
    dq 0x0
    db 'isr82', 0

    extern isr83
    align 8
    dq isr83
    dq 0x0
    db 'isr83', 0

    extern isr84
    align 8
    dq isr84
    dq 0x0
    db 'isr84', 0

    extern isr85
    align 8
    dq isr85
    dq 0x0
    db 'isr85', 0

    extern isr86
    align 8
    dq isr86
    dq 0x0
    db 'isr86', 0

    extern isr87
    align 8
    dq isr87
    dq 0x0
    db 'isr87', 0

    extern isr88
    align 8
    dq isr88
    dq 0x0
    db 'isr88', 0

    extern isr89
    align 8
    dq isr89
    dq 0x0
    db 'isr89', 0

    extern isr9
    align 8
    dq isr9
    dq 0x0
    db 'isr9', 0

    extern isr90
    align 8
    dq isr90
    dq 0x0
    db 'isr90', 0

    extern isr91
    align 8
    dq isr91
    dq 0x0
    db 'isr91', 0

    extern isr92
    align 8
    dq isr92
    dq 0x0
    db 'isr92', 0

    extern isr93
    align 8
    dq isr93
    dq 0x0
    db 'isr93', 0

    extern isr94
    align 8
    dq isr94
    dq 0x0
    db 'isr94', 0

    extern isr95
    align 8
    dq isr95
    dq 0x0
    db 'isr95', 0

    extern isr96
    align 8
    dq isr96
    dq 0x0
    db 'isr96', 0

    extern isr97
    align 8
    dq isr97
    dq 0x0
    db 'isr97', 0

    extern isr98
    align 8
    dq isr98
    dq 0x0
    db 'isr98', 0

    extern isr99
    align 8
    dq isr99
    dq 0x0
    db 'isr99', 0

    extern kcalloc
    align 8
    dq kcalloc
    dq 0xb3
    db 'kcalloc', 0

    extern kentry
    align 8
    dq kentry
    dq 0x2e2
    db 'kentry', 0

    extern kentry_threaded
    align 8
    dq kentry_threaded
    dq 0x521
    db 'kentry_threaded', 0

    extern kernel_address_req
    align 8
    dq kernel_address_req
    dq 0x8
    db 'kernel_address_req', 0

    extern kfree
    align 8
    dq kfree
    dq 0x5c
    db 'kfree', 0

    extern kmalloc
    align 8
    dq kmalloc
    dq 0x89
    db 'kmalloc', 0

    extern kopen
    align 8
    dq kopen
    dq 0x1aa
    db 'kopen', 0

    extern krealloc
    align 8
    dq krealloc
    dq 0x2c6
    db 'krealloc', 0

    extern ksleep
    align 8
    dq ksleep
    dq 0x36
    db 'ksleep', 0

    extern lai_acpi_reset
    align 8
    dq lai_acpi_reset
    dq 0x12a
    db 'lai_acpi_reset', 0

    extern lai_amlname_done
    align 8
    dq lai_amlname_done
    dq 0xe
    db 'lai_amlname_done', 0

    extern lai_amlname_iterate
    align 8
    dq lai_amlname_iterate
    dq 0x42
    db 'lai_amlname_iterate', 0

    extern lai_amlname_parse
    align 8
    dq lai_amlname_parse
    dq 0xf2
    db 'lai_amlname_parse', 0

    extern lai_api_error_to_string
    align 8
    dq lai_api_error_to_string
    dq 0x18
    db 'lai_api_error_to_string', 0

    extern lai_calloc
    align 8
    dq lai_calloc
    dq 0x33
    db 'lai_calloc', 0

    extern lai_check_device_pnp_id
    align 8
    dq lai_check_device_pnp_id
    dq 0x13e
    db 'lai_check_device_pnp_id', 0

    extern lai_create_buffer
    align 8
    dq lai_create_buffer
    dq 0x78
    db 'lai_create_buffer', 0

    extern lai_create_c_string
    align 8
    dq lai_create_c_string
    dq 0xb7
    db 'lai_create_c_string', 0

    extern lai_create_namespace
    align 8
    dq lai_create_namespace
    dq 0x2d2
    db 'lai_create_namespace', 0

    extern lai_create_nsnode
    align 8
    dq lai_create_nsnode
    dq 0x2b
    db 'lai_create_nsnode', 0

    extern lai_create_nsnode_or_die
    align 8
    dq lai_create_nsnode_or_die
    dq 0x35
    db 'lai_create_nsnode_or_die', 0

    extern lai_create_pkg
    align 8
    dq lai_create_pkg
    dq 0x80
    db 'lai_create_pkg', 0

    extern lai_create_root
    align 8
    dq lai_create_root
    dq 0x326
    db 'lai_create_root', 0

    extern lai_create_string
    align 8
    dq lai_create_string
    dq 0x7c
    db 'lai_create_string', 0

    extern lai_current_instance
    align 8
    dq lai_current_instance
    dq 0x8
    db 'lai_current_instance', 0

    extern lai_debug
    align 8
    dq lai_debug
    dq 0x80
    db 'lai_debug', 0

    extern lai_disable_acpi
    align 8
    dq lai_disable_acpi
    dq 0xc0
    db 'lai_disable_acpi', 0

    extern lai_do_os_method
    align 8
    dq lai_do_os_method
    dq 0x3b
    db 'lai_do_os_method', 0

    extern lai_do_osi_method
    align 8
    dq lai_do_osi_method
    dq 0x177
    db 'lai_do_osi_method', 0

    extern lai_do_resolve
    align 8
    dq lai_do_resolve
    dq 0x1dc
    db 'lai_do_resolve', 0

    extern lai_do_resolve_new_node
    align 8
    dq lai_do_resolve_new_node
    dq 0x1b3
    db 'lai_do_resolve_new_node', 0

    extern lai_do_rev_method
    align 8
    dq lai_do_rev_method
    dq 0x26
    db 'lai_do_rev_method', 0

    extern lai_eisaid
    align 8
    dq lai_eisaid
    dq 0x17b
    db 'lai_eisaid', 0

    extern lai_enable_acpi
    align 8
    dq lai_enable_acpi
    dq 0x1ea
    db 'lai_enable_acpi', 0

    extern lai_enable_tracing
    align 8
    dq lai_enable_tracing
    dq 0xf
    db 'lai_enable_tracing', 0

    extern lai_enter_sleep
    align 8
    dq lai_enter_sleep
    dq 0x3c5
    db 'lai_enter_sleep', 0

    extern lai_eval
    align 8
    dq lai_eval
    dq 0xa
    db 'lai_eval', 0

    extern lai_eval_args
    align 8
    dq lai_eval_args
    dq 0x456
    db 'lai_eval_args', 0

    extern lai_eval_largs
    align 8
    dq lai_eval_largs
    dq 0x42
    db 'lai_eval_largs', 0

    extern lai_eval_vargs
    align 8
    dq lai_eval_vargs
    dq 0x205
    db 'lai_eval_vargs', 0

    extern lai_evaluate_sta
    align 8
    dq lai_evaluate_sta
    dq 0xd7
    db 'lai_evaluate_sta', 0

    extern lai_exec_access
    align 8
    dq lai_exec_access
    dq 0xde
    db 'lai_exec_access', 0

    extern lai_exec_get_integer
    align 8
    dq lai_exec_get_integer
    dq 0x83
    db 'lai_exec_get_integer', 0

    extern lai_exec_get_objectref
    align 8
    dq lai_exec_get_objectref
    dq 0x21
    db 'lai_exec_get_objectref', 0

    extern lai_exec_mutate_ns
    align 8
    dq lai_exec_mutate_ns
    dq 0x294
    db 'lai_exec_mutate_ns', 0

    extern lai_exec_pkg_var_load
    align 8
    dq lai_exec_pkg_var_load
    dq 0x14
    db 'lai_exec_pkg_var_load', 0

    extern lai_exec_pkg_var_store
    align 8
    dq lai_exec_pkg_var_store
    dq 0x17
    db 'lai_exec_pkg_var_store', 0

    extern lai_exec_ref_load
    align 8
    dq lai_exec_ref_load
    dq 0x66
    db 'lai_exec_ref_load', 0

    extern lai_exec_ref_store
    align 8
    dq lai_exec_ref_store
    dq 0x66
    db 'lai_exec_ref_store', 0

    extern lai_exec_string_length
    align 8
    dq lai_exec_string_length
    dq 0x22
    db 'lai_exec_string_length', 0

    extern lai_finalize_state
    align 8
    dq lai_finalize_state
    dq 0x11e
    db 'lai_finalize_state', 0

    extern lai_get_sci_event
    align 8
    dq lai_get_sci_event
    dq 0x97
    db 'lai_get_sci_event', 0

    extern lai_init_children
    align 8
    dq lai_init_children
    dq 0x111
    db 'lai_init_children', 0

    extern lai_init_state
    align 8
    dq lai_init_state
    dq 0x65
    db 'lai_init_state', 0

    extern lai_install_nsnode
    align 8
    dq lai_install_nsnode
    dq 0x347
    db 'lai_install_nsnode', 0

    extern lai_is_name
    align 8
    dq lai_is_name
    dq 0x37
    db 'lai_is_name', 0

    extern lai_mutate_buffer
    align 8
    dq lai_mutate_buffer
    dq 0x166
    db 'lai_mutate_buffer', 0

    extern lai_mutate_integer
    align 8
    dq lai_mutate_integer
    dq 0x111
    db 'lai_mutate_integer', 0

    extern lai_mutate_string
    align 8
    dq lai_mutate_string
    dq 0x29a
    db 'lai_mutate_string', 0

    extern lai_ns_child_iterate
    align 8
    dq lai_ns_child_iterate
    dq 0x33
    db 'lai_ns_child_iterate', 0

    extern lai_ns_get_child
    align 8
    dq lai_ns_get_child
    dq 0x16e
    db 'lai_ns_get_child', 0

    extern lai_ns_get_node_type
    align 8
    dq lai_ns_get_node_type
    dq 0x32
    db 'lai_ns_get_node_type', 0

    extern lai_ns_get_opregion_address_space
    align 8
    dq lai_ns_get_opregion_address_space
    dq 0x1a
    db 'lai_ns_get_opregion_address_space', 0

    extern lai_ns_get_parent
    align 8
    dq lai_ns_get_parent
    dq 0x5
    db 'lai_ns_get_parent', 0

    extern lai_ns_get_root
    align 8
    dq lai_ns_get_root
    dq 0x8
    db 'lai_ns_get_root', 0

    extern lai_ns_iterate
    align 8
    dq lai_ns_iterate
    dq 0x3c
    db 'lai_ns_iterate', 0

    extern lai_ns_override_notify
    align 8
    dq lai_ns_override_notify
    dq 0x20
    db 'lai_ns_override_notify', 0

    extern lai_ns_override_opregion
    align 8
    dq lai_ns_override_opregion
    dq 0x3a
    db 'lai_ns_override_opregion', 0

    extern lai_obj_clone
    align 8
    dq lai_obj_clone
    dq 0x2c8
    db 'lai_obj_clone', 0

    extern lai_obj_exec_match_op
    align 8
    dq lai_obj_exec_match_op
    dq 0x261
    db 'lai_obj_exec_match_op', 0

    extern lai_obj_get_handle
    align 8
    dq lai_obj_get_handle
    dq 0x7e
    db 'lai_obj_get_handle', 0

    extern lai_obj_get_integer
    align 8
    dq lai_obj_get_integer
    dq 0x2b
    db 'lai_obj_get_integer', 0

    extern lai_obj_get_pkg
    align 8
    dq lai_obj_get_pkg
    dq 0x2e
    db 'lai_obj_get_pkg', 0

    extern lai_obj_get_type
    align 8
    dq lai_obj_get_type
    dq 0xa0
    db 'lai_obj_get_type', 0

    extern lai_obj_resize_buffer
    align 8
    dq lai_obj_resize_buffer
    dq 0x80
    db 'lai_obj_resize_buffer', 0

    extern lai_obj_resize_pkg
    align 8
    dq lai_obj_resize_pkg
    dq 0x10d
    db 'lai_obj_resize_pkg', 0

    extern lai_obj_resize_string
    align 8
    dq lai_obj_resize_string
    dq 0x84
    db 'lai_obj_resize_string', 0

    extern lai_obj_to_buffer
    align 8
    dq lai_obj_to_buffer
    dq 0x1ef
    db 'lai_obj_to_buffer', 0

    extern lai_obj_to_decimal_string
    align 8
    dq lai_obj_to_decimal_string
    dq 0x214
    db 'lai_obj_to_decimal_string', 0

    extern lai_obj_to_hex_string
    align 8
    dq lai_obj_to_hex_string
    dq 0x21c
    db 'lai_obj_to_hex_string', 0

    extern lai_obj_to_integer
    align 8
    dq lai_obj_to_integer
    dq 0x190
    db 'lai_obj_to_integer', 0

    extern lai_obj_to_string
    align 8
    dq lai_obj_to_string
    dq 0x231
    db 'lai_obj_to_string', 0

    extern lai_obj_to_type_string
    align 8
    dq lai_obj_to_type_string
    dq 0x4d8
    db 'lai_obj_to_type_string', 0

    extern lai_objecttype_ns
    align 8
    dq lai_objecttype_ns
    dq 0x69
    db 'lai_objecttype_ns', 0

    extern lai_objecttype_obj
    align 8
    dq lai_objecttype_obj
    dq 0x17
    db 'lai_objecttype_obj', 0

    extern lai_operand_emplace
    align 8
    dq lai_operand_emplace
    dq 0x26a
    db 'lai_operand_emplace', 0

    extern lai_operand_load
    align 8
    dq lai_operand_load
    dq 0xae
    db 'lai_operand_load', 0

    extern lai_operand_mutate
    align 8
    dq lai_operand_mutate
    dq 0x274
    db 'lai_operand_mutate', 0

    extern lai_panic
    align 8
    dq lai_panic
    dq 0x90
    db 'lai_panic', 0

    extern lai_pci_find_bus
    align 8
    dq lai_pci_find_bus
    dq 0x32f
    db 'lai_pci_find_bus', 0

    extern lai_pci_find_device
    align 8
    dq lai_pci_find_device
    dq 0x14b
    db 'lai_pci_find_device', 0

    extern lai_pci_parse_prt
    align 8
    dq lai_pci_parse_prt
    dq 0x527
    db 'lai_pci_parse_prt', 0

    extern lai_pci_route
    align 8
    dq lai_pci_route
    dq 0x62
    db 'lai_pci_route', 0

    extern lai_pci_route_pin
    align 8
    dq lai_pci_route_pin
    dq 0x1ce
    db 'lai_pci_route_pin', 0

    extern lai_populate
    align 8
    dq lai_populate
    dq 0x2af
    db 'lai_populate', 0

    extern lai_read_bankfield
    align 8
    dq lai_read_bankfield
    dq 0x86
    db 'lai_read_bankfield', 0

    extern lai_read_field
    align 8
    dq lai_read_field
    dq 0x1b2
    db 'lai_read_field', 0

    extern lai_read_field_internal
    align 8
    dq lai_read_field_internal
    dq 0x1ff
    db 'lai_read_field_internal', 0

    extern lai_read_opregion
    align 8
    dq lai_read_opregion
    dq 0xc9
    db 'lai_read_opregion', 0

    extern lai_read_resource
    align 8
    dq lai_read_resource
    dq 0x1bb
    db 'lai_read_resource', 0

    extern lai_resolve_new_node
    align 8
    dq lai_resolve_new_node
    dq 0x112
    db 'lai_resolve_new_node', 0

    extern lai_resolve_path
    align 8
    dq lai_resolve_path
    dq 0x1fb
    db 'lai_resolve_path', 0

    extern lai_resolve_search
    align 8
    dq lai_resolve_search
    dq 0x7f
    db 'lai_resolve_search', 0

    extern lai_resource_get_type
    align 8
    dq lai_resource_get_type
    dq 0xa9
    db 'lai_resource_get_type', 0

    extern lai_resource_irq_is_active_low
    align 8
    dq lai_resource_irq_is_active_low
    dq 0x6e
    db 'lai_resource_irq_is_active_low', 0

    extern lai_resource_irq_is_level_triggered
    align 8
    dq lai_resource_irq_is_level_triggered
    dq 0x7b
    db 'lai_resource_irq_is_level_triggered', 0

    extern lai_resource_iterate
    align 8
    dq lai_resource_iterate
    dq 0x145
    db 'lai_resource_iterate', 0

    extern lai_resource_next_irq
    align 8
    dq lai_resource_next_irq
    dq 0xb5
    db 'lai_resource_next_irq', 0

    extern lai_set_acpi_revision
    align 8
    dq lai_set_acpi_revision
    dq 0x11
    db 'lai_set_acpi_revision', 0

    extern lai_set_sci_event
    align 8
    dq lai_set_sci_event
    dq 0x9a
    db 'lai_set_sci_event', 0

    extern lai_snprintf
    align 8
    dq lai_snprintf
    dq 0x42
    db 'lai_snprintf', 0

    extern lai_store_ns
    align 8
    dq lai_store_ns
    dq 0xe4
    db 'lai_store_ns', 0

    extern lai_strcmp
    align 8
    dq lai_strcmp
    dq 0x4a
    db 'lai_strcmp', 0

    extern lai_strcpy
    align 8
    dq lai_strcpy
    dq 0x35
    db 'lai_strcpy', 0

    extern lai_stringify_amlname
    align 8
    dq lai_stringify_amlname
    dq 0x165
    db 'lai_stringify_amlname', 0

    extern lai_stringify_node_path
    align 8
    dq lai_stringify_node_path
    dq 0xf5
    db 'lai_stringify_node_path', 0

    extern lai_strlen
    align 8
    dq lai_strlen
    dq 0x22
    db 'lai_strlen', 0

    extern lai_swap_object
    align 8
    dq lai_swap_object
    dq 0x9f
    db 'lai_swap_object', 0

    extern lai_uninstall_nsnode
    align 8
    dq lai_uninstall_nsnode
    dq 0x2dc
    db 'lai_uninstall_nsnode', 0

    extern lai_var_assign
    align 8
    dq lai_var_assign
    dq 0x143
    db 'lai_var_assign', 0

    extern lai_var_finalize
    align 8
    dq lai_var_finalize
    dq 0x107
    db 'lai_var_finalize', 0

    extern lai_var_move
    align 8
    dq lai_var_move
    dq 0x11d
    db 'lai_var_move', 0

    extern lai_vsnprintf
    align 8
    dq lai_vsnprintf
    dq 0x953
    db 'lai_vsnprintf', 0

    extern lai_warn
    align 8
    dq lai_warn
    dq 0x80
    db 'lai_warn', 0

    extern lai_write_bankfield
    align 8
    dq lai_write_bankfield
    dq 0x86
    db 'lai_write_bankfield', 0

    extern lai_write_field
    align 8
    dq lai_write_field
    dq 0x79
    db 'lai_write_field', 0

    extern lai_write_field_internal
    align 8
    dq lai_write_field_internal
    dq 0x7a3
    db 'lai_write_field_internal', 0

    extern lai_write_opregion
    align 8
    dq lai_write_opregion
    dq 0xc9
    db 'lai_write_opregion', 0

    extern laihost_free
    align 8
    dq laihost_free
    dq 0xa
    db 'laihost_free', 0

    extern laihost_inb
    align 8
    dq laihost_inb
    dq 0x9
    db 'laihost_inb', 0

    extern laihost_ind
    align 8
    dq laihost_ind
    dq 0xc
    db 'laihost_ind', 0

    extern laihost_inw
    align 8
    dq laihost_inw
    dq 0xd
    db 'laihost_inw', 0

    extern laihost_log
    align 8
    dq laihost_log
    dq 0x13
    db 'laihost_log', 0

    extern laihost_malloc
    align 8
    dq laihost_malloc
    dq 0xa
    db 'laihost_malloc', 0

    extern laihost_map
    align 8
    dq laihost_map
    dq 0x13
    db 'laihost_map', 0

    extern laihost_outb
    align 8
    dq laihost_outb
    dq 0xb
    db 'laihost_outb', 0

    extern laihost_outd
    align 8
    dq laihost_outd
    dq 0xb
    db 'laihost_outd', 0

    extern laihost_outw
    align 8
    dq laihost_outw
    dq 0xc
    db 'laihost_outw', 0

    extern laihost_panic
    align 8
    dq laihost_panic
    dq 0x1a
    db 'laihost_panic', 0

    extern laihost_pci_readb
    align 8
    dq laihost_pci_readb
    dq 0x11
    db 'laihost_pci_readb', 0

    extern laihost_pci_readd
    align 8
    dq laihost_pci_readd
    dq 0x10
    db 'laihost_pci_readd', 0

    extern laihost_pci_readw
    align 8
    dq laihost_pci_readw
    dq 0x11
    db 'laihost_pci_readw', 0

    extern laihost_pci_writeb
    align 8
    dq laihost_pci_writeb
    dq 0x1a
    db 'laihost_pci_writeb', 0

    extern laihost_pci_writed
    align 8
    dq laihost_pci_writed
    dq 0x1a
    db 'laihost_pci_writed', 0

    extern laihost_pci_writew
    align 8
    dq laihost_pci_writew
    dq 0x1a
    db 'laihost_pci_writew', 0

    extern laihost_realloc
    align 8
    dq laihost_realloc
    dq 0xa
    db 'laihost_realloc', 0

    extern laihost_scan
    align 8
    dq laihost_scan
    dq 0xa
    db 'laihost_scan', 0

    extern laihost_sleep
    align 8
    dq laihost_sleep
    dq 0xa
    db 'laihost_sleep', 0

    extern laihost_timer
    align 8
    dq laihost_timer
    dq 0x25
    db 'laihost_timer', 0

    extern laihost_unmap
    align 8
    dq laihost_unmap
    dq 0x6
    db 'laihost_unmap', 0

    extern lapic_read
    align 8
    dq lapic_read
    dq 0x2a
    db 'lapic_read', 0

    extern lapic_write
    align 8
    dq lapic_write
    dq 0x27
    db 'lapic_write', 0

    extern lfind
    align 8
    dq lfind
    dq 0x3a
    db 'lfind', 0

    extern limine_kernel_address_request
    align 8
    dq limine_kernel_address_request
    dq 0x30
    db 'limine_kernel_address_request', 0

    extern limine_memmap_request
    align 8
    dq limine_memmap_request
    dq 0x30
    db 'limine_memmap_request', 0

    extern limine_rsdp_request
    align 8
    dq limine_rsdp_request
    dq 0x30
    db 'limine_rsdp_request', 0

    extern limine_stack_size_request
    align 8
    dq limine_stack_size_request
    dq 0x38
    db 'limine_stack_size_request', 0

    extern list_contains
    align 8
    dq list_contains
    dq 0x21f
    db 'list_contains', 0

    extern list_create
    align 8
    dq list_create
    dq 0x194
    db 'list_create', 0

    extern list_destroy
    align 8
    dq list_destroy
    dq 0xb7
    db 'list_destroy', 0

    extern list_destroy_node
    align 8
    dq list_destroy_node
    dq 0xa
    db 'list_destroy_node', 0

    extern list_get_back
    align 8
    dq list_get_back
    dq 0x240
    db 'list_get_back', 0

    extern list_get_front
    align 8
    dq list_get_front
    dq 0x240
    db 'list_get_front', 0

    extern list_get_indexed
    align 8
    dq list_get_indexed
    dq 0x20c
    db 'list_get_indexed', 0

    extern list_pop
    align 8
    dq list_pop
    dq 0x3ee
    db 'list_pop', 0

    extern list_push_back
    align 8
    dq list_push_back
    dq 0x4b9
    db 'list_push_back', 0

    extern list_push_before
    align 8
    dq list_push_before
    dq 0x445
    db 'list_push_before', 0

    extern list_push_front
    align 8
    dq list_push_front
    dq 0x431
    db 'list_push_front', 0

    extern list_remove
    align 8
    dq list_remove
    dq 0x4c2
    db 'list_remove', 0

    extern list_remove_back
    align 8
    dq list_remove_back
    dq 0x45a
    db 'list_remove_back', 0

    extern list_remove_front
    align 8
    dq list_remove_front
    dq 0x45a
    db 'list_remove_front', 0

    extern list_remove_indexed
    align 8
    dq list_remove_indexed
    dq 0x1f
    db 'list_remove_indexed', 0

    extern memcmp
    align 8
    dq memcmp
    dq 0x14a
    db 'memcmp', 0

    extern memcpy
    align 8
    dq memcpy
    dq 0x123
    db 'memcpy', 0

    extern memcpy64
    align 8
    dq memcpy64
    dq 0x1b4
    db 'memcpy64', 0

    extern memmap_req
    align 8
    dq memmap_req
    dq 0x8
    db 'memmap_req', 0

    extern memmove
    align 8
    dq memmove
    dq 0x1ae
    db 'memmove', 0

    extern memset
    align 8
    dq memset
    dq 0xa9
    db 'memset', 0

    extern module_load
    align 8
    dq module_load
    dq 0x142e
    db 'module_load', 0

    extern modules_init
    align 8
    dq modules_init
    dq 0x128
    db 'modules_init', 0

    extern nvme_create_queue_pair
    align 8
    dq nvme_create_queue_pair
    dq 0x88a
    db 'nvme_create_queue_pair', 0

    extern nvme_execute_read_write
    align 8
    dq nvme_execute_read_write
    dq 0x135a
    db 'nvme_execute_read_write', 0

    extern nvme_get_ns_info
    align 8
    dq nvme_get_ns_info
    dq 0x117
    db 'nvme_get_ns_info', 0

    extern nvme_identify
    align 8
    dq nvme_identify
    dq 0x345
    db 'nvme_identify', 0

    extern nvme_init
    align 8
    dq nvme_init
    dq 0xeda
    db 'nvme_init', 0

    extern nvme_lseek
    align 8
    dq nvme_lseek
    dq 0x605
    db 'nvme_lseek', 0

    extern nvme_read
    align 8
    dq nvme_read
    dq 0x7e0
    db 'nvme_read', 0

    extern nvme_send_command
    align 8
    dq nvme_send_command
    dq 0x248
    db 'nvme_send_command', 0

    extern nvme_send_command_and_wait
    align 8
    dq nvme_send_command_and_wait
    dq 0x44d
    db 'nvme_send_command_and_wait', 0

    extern nvme_set_queue_count
    align 8
    dq nvme_set_queue_count
    dq 0x1d5
    db 'nvme_set_queue_count', 0

    extern nvme_write
    align 8
    dq nvme_write
    dq 0x90b
    db 'nvme_write', 0

    extern partfs_init
    align 8
    dq partfs_init
    dq 0x7ac
    db 'partfs_init', 0

    extern partfs_lseek
    align 8
    dq partfs_lseek
    dq 0x423
    db 'partfs_lseek', 0

    extern partfs_read
    align 8
    dq partfs_read
    dq 0x51a
    db 'partfs_read', 0

    extern partfs_write
    align 8
    dq partfs_write
    dq 0x51a
    db 'partfs_write', 0

    extern pciIrqPins
    align 8
    dq pciIrqPins
    dq 0xa00
    db 'pciIrqPins', 0

    extern pciIrqPinsData
    align 8
    dq pciIrqPinsData
    dq 0xa00
    db 'pciIrqPinsData', 0

    extern pci_device_read_config
    align 8
    dq pci_device_read_config
    dq 0x1b5
    db 'pci_device_read_config', 0

    extern pci_device_write_config
    align 8
    dq pci_device_write_config
    dq 0x1bf
    db 'pci_device_write_config', 0

    extern pci_enable_busmastering
    align 8
    dq pci_enable_busmastering
    dq 0x40
    db 'pci_enable_busmastering', 0

    extern pci_enable_mmio
    align 8
    dq pci_enable_mmio
    dq 0x35
    db 'pci_enable_mmio', 0

    extern pci_get_device
    align 8
    dq pci_get_device
    dq 0x3fe
    db 'pci_get_device', 0

    extern pci_get_device_by_vendor
    align 8
    dq pci_get_device_by_vendor
    dq 0x3d6
    db 'pci_get_device_by_vendor', 0

    extern pci_init
    align 8
    dq pci_init
    dq 0xebf
    db 'pci_init', 0

    extern pci_preinit
    align 8
    dq pci_preinit
    dq 0x164
    db 'pci_preinit', 0

    extern pci_read_bar
    align 8
    dq pci_read_bar
    dq 0x183
    db 'pci_read_bar', 0

    extern pci_read_config
    align 8
    dq pci_read_config
    dq 0x323
    db 'pci_read_config', 0

    extern pci_set_interrupts
    align 8
    dq pci_set_interrupts
    dq 0x41
    db 'pci_set_interrupts', 0

    extern pci_write_config
    align 8
    dq pci_write_config
    dq 0x373
    db 'pci_write_config', 0

    extern pipe_close
    align 8
    dq pipe_close
    dq 0x44a
    db 'pipe_close', 0

    extern pipe_create
    align 8
    dq pipe_create
    dq 0x5c4
    db 'pipe_create', 0

    extern pipe_read
    align 8
    dq pipe_read
    dq 0xb75
    db 'pipe_read', 0

    extern pipe_set_read_at_first_data
    align 8
    dq pipe_set_read_at_first_data
    dq 0xa8
    db 'pipe_set_read_at_first_data', 0

    extern pipe_write
    align 8
    dq pipe_write
    dq 0x59b
    db 'pipe_write', 0

    extern pmm_alloc
    align 8
    dq pmm_alloc
    dq 0x6d
    db 'pmm_alloc', 0

    extern pmm_alloc_advanced
    align 8
    dq pmm_alloc_advanced
    dq 0xfe
    db 'pmm_alloc_advanced', 0

    extern pmm_free
    align 8
    dq pmm_free
    dq 0xba
    db 'pmm_free', 0

    extern pmm_init
    align 8
    dq pmm_init
    dq 0x933
    db 'pmm_init', 0

    extern printf
    align 8
    dq printf
    dq 0xe3
    db 'printf', 0

    extern printf_scheduler
    align 8
    dq printf_scheduler
    dq 0xe3
    db 'printf_scheduler', 0

    extern printf_write_char_e8
    align 8
    dq printf_write_char_e8
    dq 0x164
    db 'printf_write_char_e8', 0

    extern printf_write_char_str
    align 8
    dq printf_write_char_str
    dq 0x300
    db 'printf_write_char_str', 0

    extern process_create
    align 8
    dq process_create
    dq 0x390
    db 'process_create', 0

    extern qemu_debug_puts_urgent
    align 8
    dq qemu_debug_puts_urgent
    dq 0xc6
    db 'qemu_debug_puts_urgent', 0

    extern rsdp_req
    align 8
    dq rsdp_req
    dq 0x8
    db 'rsdp_req', 0

    extern scheduler_add_task
    align 8
    dq scheduler_add_task
    dq 0x2f2
    db 'scheduler_add_task', 0

    extern scheduler_get_current_pid
    align 8
    dq scheduler_get_current_pid
    dq 0xc4
    db 'scheduler_get_current_pid', 0

    extern scheduler_init
    align 8
    dq scheduler_init
    dq 0x151
    db 'scheduler_init', 0

    extern scheduler_schedule
    align 8
    dq scheduler_schedule
    dq 0xadb
    db 'scheduler_schedule', 0

    extern scheduler_schedule_smp
    align 8
    dq scheduler_schedule_smp
    dq 0xc2
    db 'scheduler_schedule_smp', 0

    extern scheduler_switch_task
    align 8
    dq scheduler_switch_task
    dq 0x0
    db 'scheduler_switch_task', 0

    extern smp_init
    align 8
    dq smp_init
    dq 0x37c
    db 'smp_init', 0

    extern smp_init_cpu_misc
    align 8
    dq smp_init_cpu_misc
    dq 0x6f
    db 'smp_init_cpu_misc', 0

    extern smp_init_trampoline
    align 8
    dq smp_init_trampoline
    dq 0x0
    db 'smp_init_trampoline', 0

    extern sprintf
    align 8
    dq sprintf
    dq 0xd3
    db 'sprintf', 0

    extern stack_size_req
    align 8
    dq stack_size_req
    dq 0x8
    db 'stack_size_req', 0

    extern strchr
    align 8
    dq strchr
    dq 0x3a
    db 'strchr', 0

    extern strchrnul
    align 8
    dq strchrnul
    dq 0x31e
    db 'strchrnul', 0

    extern strcmp
    align 8
    dq strcmp
    dq 0x113
    db 'strcmp', 0

    extern strcpy
    align 8
    dq strcpy
    dq 0x16d
    db 'strcpy', 0

    extern strcspn
    align 8
    dq strcspn
    dq 0x221
    db 'strcspn', 0

    extern strdup
    align 8
    dq strdup
    dq 0x151
    db 'strdup', 0

    extern strlen
    align 8
    dq strlen
    dq 0x8a
    db 'strlen', 0

    extern strncmp
    align 8
    dq strncmp
    dq 0x15e
    db 'strncmp', 0

    extern strpbrk
    align 8
    dq strpbrk
    dq 0x68
    db 'strpbrk', 0

    extern strsep
    align 8
    dq strsep
    dq 0x1dd
    db 'strsep', 0

    extern strspn
    align 8
    dq strspn
    dq 0x28d
    db 'strspn', 0

    extern strstr
    align 8
    dq strstr
    dq 0x17a
    db 'strstr', 0

    extern strtok_r
    align 8
    dq strtok_r
    dq 0x217
    db 'strtok_r', 0

    extern sym_resolve
    align 8
    dq sym_resolve
    dq 0x13c
    db 'sym_resolve', 0

    extern syscall_alloc
    align 8
    dq syscall_alloc
    dq 0x3c0
    db 'syscall_alloc', 0

    extern syscall_close
    align 8
    dq syscall_close
    dq 0x3b8
    db 'syscall_close', 0

    extern syscall_debug
    align 8
    dq syscall_debug
    dq 0x27b
    db 'syscall_debug', 0

    extern syscall_entry
    align 8
    dq syscall_entry
    dq 0x0
    db 'syscall_entry', 0

    extern syscall_fork
    align 8
    dq syscall_fork
    dq 0x811
    db 'syscall_fork', 0

    extern syscall_install
    align 8
    dq syscall_install
    dq 0x10a
    db 'syscall_install', 0

    extern syscall_ioctl
    align 8
    dq syscall_ioctl
    dq 0x3f3
    db 'syscall_ioctl', 0

    extern syscall_module_or_invalid
    align 8
    dq syscall_module_or_invalid
    dq 0x231
    db 'syscall_module_or_invalid', 0

    extern syscall_open
    align 8
    dq syscall_open
    dq 0x4d9
    db 'syscall_open', 0

    extern syscall_privilege_check
    align 8
    dq syscall_privilege_check
    dq 0x14
    db 'syscall_privilege_check', 0

    extern syscall_read
    align 8
    dq syscall_read
    dq 0x443
    db 'syscall_read', 0

    extern syscall_seek
    align 8
    dq syscall_seek
    dq 0x3f3
    db 'syscall_seek', 0

    extern syscall_tcb_set
    align 8
    dq syscall_tcb_set
    dq 0x1d8
    db 'syscall_tcb_set', 0

    extern syscall_write
    align 8
    dq syscall_write
    dq 0x443
    db 'syscall_write', 0

    extern temp_unwind
    align 8
    dq temp_unwind
    dq 0xd
    db 'temp_unwind', 0

    extern temp_unwind_from
    align 8
    dq temp_unwind_from
    dq 0xb9
    db 'temp_unwind_from', 0

    extern thread_create
    align 8
    dq thread_create
    dq 0x193a
    db 'thread_create', 0

    extern thread_sleep
    align 8
    dq thread_sleep
    dq 0x179
    db 'thread_sleep', 0

    extern timer_get_nanoseconds
    align 8
    dq timer_get_nanoseconds
    dq 0x79
    db 'timer_get_nanoseconds', 0

    extern timer_handler
    align 8
    dq timer_handler
    dq 0xc3
    db 'timer_handler', 0

    extern timer_init
    align 8
    dq timer_init
    dq 0x2b9
    db 'timer_init', 0

    extern vfs_close
    align 8
    dq vfs_close
    dq 0x150
    db 'vfs_close', 0

    extern vfs_create
    align 8
    dq vfs_create
    dq 0x236
    db 'vfs_create', 0

    extern vfs_finddir
    align 8
    dq vfs_finddir
    dq 0x236
    db 'vfs_finddir', 0

    extern vfs_get_mountpoint
    align 8
    dq vfs_get_mountpoint
    dq 0xe4b
    db 'vfs_get_mountpoint', 0

    extern vfs_init
    align 8
    dq vfs_init
    dq 0xf6
    db 'vfs_init', 0

    extern vfs_ioctl
    align 8
    dq vfs_ioctl
    dq 0x1c2
    db 'vfs_ioctl', 0

    extern vfs_lseek
    align 8
    dq vfs_lseek
    dq 0x1c2
    db 'vfs_lseek', 0

    extern vfs_mount
    align 8
    dq vfs_mount
    dq 0x2c6
    db 'vfs_mount', 0

    extern vfs_node_mount
    align 8
    dq vfs_node_mount
    dq 0x2bf
    db 'vfs_node_mount', 0

    extern vfs_open
    align 8
    dq vfs_open
    dq 0x13c
    db 'vfs_open', 0

    extern vfs_print_tree
    align 8
    dq vfs_print_tree
    dq 0x57
    db 'vfs_print_tree', 0

    extern vfs_read
    align 8
    dq vfs_read
    dq 0x1c2
    db 'vfs_read', 0

    extern vfs_readdir
    align 8
    dq vfs_readdir
    dq 0x1f6
    db 'vfs_readdir', 0

    extern vfs_write
    align 8
    dq vfs_write
    dq 0x1c2
    db 'vfs_write', 0

    extern vmm_copy_pml4
    align 8
    dq vmm_copy_pml4
    dq 0xa43
    db 'vmm_copy_pml4', 0

    extern vmm_free_pml4
    align 8
    dq vmm_free_pml4
    dq 0x5c1
    db 'vmm_free_pml4', 0

    extern vmm_get_phys
    align 8
    dq vmm_get_phys
    dq 0x575
    db 'vmm_get_phys', 0

    extern vmm_init
    align 8
    dq vmm_init
    dq 0x4b2
    db 'vmm_init', 0

    extern vmm_map
    align 8
    dq vmm_map
    dq 0xa
    db 'vmm_map', 0

    extern vmm_map_huge
    align 8
    dq vmm_map_huge
    dq 0x10
    db 'vmm_map_huge', 0

    extern vmm_map_if_not_mapped
    align 8
    dq vmm_map_if_not_mapped
    dq 0x5f1
    db 'vmm_map_if_not_mapped', 0

    extern vmm_read
    align 8
    dq vmm_read
    dq 0x678
    db 'vmm_read', 0

    extern vmm_setup_pml4
    align 8
    dq vmm_setup_pml4
    dq 0x114
    db 'vmm_setup_pml4', 0

    extern vmm_unmap
    align 8
    dq vmm_unmap
    dq 0x655
    db 'vmm_unmap', 0

    extern yield
    align 8
    dq yield
    dq 0x6a
    db 'yield', 0
global kernel_symbols_end
kernel_symbols_end: ;
