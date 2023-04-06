SECTION .symbols
global kernel_symbols_start
kernel_symbols_start:

    extern ActiveTasks
    dq ActiveTasks
    db 'ActiveTasks', 0

    extern CPULocals
    dq CPULocals
    db 'CPULocals', 0

    extern CpuSimdRegionSize
    dq CpuSimdRegionSize
    db 'CpuSimdRegionSize', 0

    extern DefaultFxstate
    dq DefaultFxstate
    db 'DefaultFxstate', 0

    extern DevFSList
    dq DevFSList
    db 'DevFSList', 0

    extern IRQFunctions
    dq IRQFunctions
    db 'IRQFunctions', 0

    extern KernelPml4
    dq KernelPml4
    db 'KernelPml4', 0

    extern KernelSymbolsHashmap
    dq KernelSymbolsHashmap
    db 'KernelSymbolsHashmap', 0

    extern NVMEDevices
    dq NVMEDevices
    db 'NVMEDevices', 0

    extern ReSchedulerLock
    dq ReSchedulerLock
    db 'ReSchedulerLock', 0

    extern SchedulerLock
    dq SchedulerLock
    db 'SchedulerLock', 0

    extern SchedulerProcesses
    dq SchedulerProcesses
    db 'SchedulerProcesses', 0

    extern SchedulerRunning
    dq SchedulerRunning
    db 'SchedulerRunning', 0

    extern TimerCounter
    dq TimerCounter
    db 'TimerCounter', 0

    extern TimerSeconds
    dq TimerSeconds
    db 'TimerSeconds', 0

    extern VFSTree
    dq VFSTree
    db 'VFSTree', 0

    extern _vsnprintf_internal
    dq _vsnprintf_internal
    db '_vsnprintf_internal', 0

    extern acpi_get_table
    dq acpi_get_table
    db 'acpi_get_table', 0

    extern acpi_init
    dq acpi_init
    db 'acpi_init', 0

    extern acpi_post_init
    dq acpi_post_init
    db 'acpi_post_init', 0

    extern bswap16
    dq bswap16
    db 'bswap16', 0

    extern bswap32
    dq bswap32
    db 'bswap32', 0

    extern bswap64
    dq bswap64
    db 'bswap64', 0

    extern char_to_hex
    dq char_to_hex
    db 'char_to_hex', 0

    extern cpu_restore_simd
    dq cpu_restore_simd
    db 'cpu_restore_simd', 0

    extern cpu_save_simd
    dq cpu_save_simd
    db 'cpu_save_simd', 0

    extern crc32_calculate_buffer
    dq crc32_calculate_buffer
    db 'crc32_calculate_buffer', 0

    extern devfs_finddir
    dq devfs_finddir
    db 'devfs_finddir', 0

    extern devfs_init
    dq devfs_init
    db 'devfs_init', 0

    extern devfs_mount
    dq devfs_mount
    db 'devfs_mount', 0

    extern devfs_print_tree
    dq devfs_print_tree
    db 'devfs_print_tree', 0

    extern dispatch_interrupt
    dq dispatch_interrupt
    db 'dispatch_interrupt', 0

    extern elf_load
    dq elf_load
    db 'elf_load', 0

    extern event_array_await_timeout
    dq event_array_await_timeout
    db 'event_array_await_timeout', 0

    extern event_await
    dq event_await
    db 'event_await', 0

    extern event_await_timeout
    dq event_await_timeout
    db 'event_await_timeout', 0

    extern event_notify
    dq event_notify
    db 'event_notify', 0

    extern event_reset
    dq event_reset
    db 'event_reset', 0

    extern events_await
    dq events_await
    db 'events_await', 0

    extern exec
    dq exec
    db 'exec', 0

    extern fat32_finddir
    dq fat32_finddir
    db 'fat32_finddir', 0

    extern fat32_init
    dq fat32_init
    db 'fat32_init', 0

    extern fat32_lseek
    dq fat32_lseek
    db 'fat32_lseek', 0

    extern fat32_mount
    dq fat32_mount
    db 'fat32_mount', 0

    extern fat32_print_tree
    dq fat32_print_tree
    db 'fat32_print_tree', 0

    extern fat32_read
    dq fat32_read
    db 'fat32_read', 0

    extern force_reschedule
    dq force_reschedule
    db 'force_reschedule', 0

    extern gdt_init
    dq gdt_init
    db 'gdt_init', 0

    extern gdt_setup_tss
    dq gdt_setup_tss
    db 'gdt_setup_tss', 0

    extern gentree_create
    dq gentree_create
    db 'gentree_create', 0

    extern gentree_get_parent
    dq gentree_get_parent
    db 'gentree_get_parent', 0

    extern gentree_insert
    dq gentree_insert
    db 'gentree_insert', 0

    extern gentree_node_create
    dq gentree_node_create
    db 'gentree_node_create', 0

    extern gentree_remove
    dq gentree_remove
    db 'gentree_remove', 0

    extern hashmap_create
    dq hashmap_create
    db 'hashmap_create', 0

    extern hashmap_delete
    dq hashmap_delete
    db 'hashmap_delete', 0

    extern hashmap_get
    dq hashmap_get
    db 'hashmap_get', 0

    extern hashmap_has
    dq hashmap_has
    db 'hashmap_has', 0

    extern hashmap_remove
    dq hashmap_remove
    db 'hashmap_remove', 0

    extern hashmap_set
    dq hashmap_set
    db 'hashmap_set', 0

    extern hashmap_string_hash
    dq hashmap_string_hash
    db 'hashmap_string_hash', 0

    extern hashmap_to_list
    dq hashmap_to_list
    db 'hashmap_to_list', 0

    extern idt_add_pci_handler
    dq idt_add_pci_handler
    db 'idt_add_pci_handler', 0

    extern idt_init
    dq idt_init
    db 'idt_init', 0

    extern ioapic_get_id_from_gsi
    dq ioapic_get_id_from_gsi
    db 'ioapic_get_id_from_gsi', 0

    extern ioapic_read
    dq ioapic_read
    db 'ioapic_read', 0

    extern ioapic_redirect_gsi
    dq ioapic_redirect_gsi
    db 'ioapic_redirect_gsi', 0

    extern ioapic_redirect_legacy_irq
    dq ioapic_redirect_legacy_irq
    db 'ioapic_redirect_legacy_irq', 0

    extern ioapic_write
    dq ioapic_write
    db 'ioapic_write', 0

    extern isr0
    dq isr0
    db 'isr0', 0

    extern isr1
    dq isr1
    db 'isr1', 0

    extern isr10
    dq isr10
    db 'isr10', 0

    extern isr100
    dq isr100
    db 'isr100', 0

    extern isr101
    dq isr101
    db 'isr101', 0

    extern isr102
    dq isr102
    db 'isr102', 0

    extern isr103
    dq isr103
    db 'isr103', 0

    extern isr104
    dq isr104
    db 'isr104', 0

    extern isr105
    dq isr105
    db 'isr105', 0

    extern isr106
    dq isr106
    db 'isr106', 0

    extern isr107
    dq isr107
    db 'isr107', 0

    extern isr108
    dq isr108
    db 'isr108', 0

    extern isr109
    dq isr109
    db 'isr109', 0

    extern isr11
    dq isr11
    db 'isr11', 0

    extern isr110
    dq isr110
    db 'isr110', 0

    extern isr111
    dq isr111
    db 'isr111', 0

    extern isr112
    dq isr112
    db 'isr112', 0

    extern isr113
    dq isr113
    db 'isr113', 0

    extern isr114
    dq isr114
    db 'isr114', 0

    extern isr115
    dq isr115
    db 'isr115', 0

    extern isr116
    dq isr116
    db 'isr116', 0

    extern isr117
    dq isr117
    db 'isr117', 0

    extern isr118
    dq isr118
    db 'isr118', 0

    extern isr119
    dq isr119
    db 'isr119', 0

    extern isr12
    dq isr12
    db 'isr12', 0

    extern isr120
    dq isr120
    db 'isr120', 0

    extern isr121
    dq isr121
    db 'isr121', 0

    extern isr122
    dq isr122
    db 'isr122', 0

    extern isr123
    dq isr123
    db 'isr123', 0

    extern isr124
    dq isr124
    db 'isr124', 0

    extern isr125
    dq isr125
    db 'isr125', 0

    extern isr126
    dq isr126
    db 'isr126', 0

    extern isr127
    dq isr127
    db 'isr127', 0

    extern isr128
    dq isr128
    db 'isr128', 0

    extern isr129
    dq isr129
    db 'isr129', 0

    extern isr13
    dq isr13
    db 'isr13', 0

    extern isr130
    dq isr130
    db 'isr130', 0

    extern isr131
    dq isr131
    db 'isr131', 0

    extern isr132
    dq isr132
    db 'isr132', 0

    extern isr133
    dq isr133
    db 'isr133', 0

    extern isr134
    dq isr134
    db 'isr134', 0

    extern isr135
    dq isr135
    db 'isr135', 0

    extern isr136
    dq isr136
    db 'isr136', 0

    extern isr137
    dq isr137
    db 'isr137', 0

    extern isr138
    dq isr138
    db 'isr138', 0

    extern isr139
    dq isr139
    db 'isr139', 0

    extern isr14
    dq isr14
    db 'isr14', 0

    extern isr140
    dq isr140
    db 'isr140', 0

    extern isr141
    dq isr141
    db 'isr141', 0

    extern isr142
    dq isr142
    db 'isr142', 0

    extern isr143
    dq isr143
    db 'isr143', 0

    extern isr144
    dq isr144
    db 'isr144', 0

    extern isr145
    dq isr145
    db 'isr145', 0

    extern isr146
    dq isr146
    db 'isr146', 0

    extern isr147
    dq isr147
    db 'isr147', 0

    extern isr148
    dq isr148
    db 'isr148', 0

    extern isr149
    dq isr149
    db 'isr149', 0

    extern isr15
    dq isr15
    db 'isr15', 0

    extern isr150
    dq isr150
    db 'isr150', 0

    extern isr151
    dq isr151
    db 'isr151', 0

    extern isr152
    dq isr152
    db 'isr152', 0

    extern isr153
    dq isr153
    db 'isr153', 0

    extern isr154
    dq isr154
    db 'isr154', 0

    extern isr155
    dq isr155
    db 'isr155', 0

    extern isr156
    dq isr156
    db 'isr156', 0

    extern isr157
    dq isr157
    db 'isr157', 0

    extern isr158
    dq isr158
    db 'isr158', 0

    extern isr159
    dq isr159
    db 'isr159', 0

    extern isr16
    dq isr16
    db 'isr16', 0

    extern isr160
    dq isr160
    db 'isr160', 0

    extern isr161
    dq isr161
    db 'isr161', 0

    extern isr162
    dq isr162
    db 'isr162', 0

    extern isr163
    dq isr163
    db 'isr163', 0

    extern isr164
    dq isr164
    db 'isr164', 0

    extern isr165
    dq isr165
    db 'isr165', 0

    extern isr166
    dq isr166
    db 'isr166', 0

    extern isr167
    dq isr167
    db 'isr167', 0

    extern isr168
    dq isr168
    db 'isr168', 0

    extern isr169
    dq isr169
    db 'isr169', 0

    extern isr17
    dq isr17
    db 'isr17', 0

    extern isr170
    dq isr170
    db 'isr170', 0

    extern isr171
    dq isr171
    db 'isr171', 0

    extern isr172
    dq isr172
    db 'isr172', 0

    extern isr173
    dq isr173
    db 'isr173', 0

    extern isr174
    dq isr174
    db 'isr174', 0

    extern isr175
    dq isr175
    db 'isr175', 0

    extern isr176
    dq isr176
    db 'isr176', 0

    extern isr177
    dq isr177
    db 'isr177', 0

    extern isr178
    dq isr178
    db 'isr178', 0

    extern isr179
    dq isr179
    db 'isr179', 0

    extern isr18
    dq isr18
    db 'isr18', 0

    extern isr180
    dq isr180
    db 'isr180', 0

    extern isr181
    dq isr181
    db 'isr181', 0

    extern isr182
    dq isr182
    db 'isr182', 0

    extern isr183
    dq isr183
    db 'isr183', 0

    extern isr184
    dq isr184
    db 'isr184', 0

    extern isr185
    dq isr185
    db 'isr185', 0

    extern isr186
    dq isr186
    db 'isr186', 0

    extern isr187
    dq isr187
    db 'isr187', 0

    extern isr188
    dq isr188
    db 'isr188', 0

    extern isr189
    dq isr189
    db 'isr189', 0

    extern isr19
    dq isr19
    db 'isr19', 0

    extern isr190
    dq isr190
    db 'isr190', 0

    extern isr191
    dq isr191
    db 'isr191', 0

    extern isr192
    dq isr192
    db 'isr192', 0

    extern isr193
    dq isr193
    db 'isr193', 0

    extern isr194
    dq isr194
    db 'isr194', 0

    extern isr195
    dq isr195
    db 'isr195', 0

    extern isr196
    dq isr196
    db 'isr196', 0

    extern isr197
    dq isr197
    db 'isr197', 0

    extern isr198
    dq isr198
    db 'isr198', 0

    extern isr199
    dq isr199
    db 'isr199', 0

    extern isr2
    dq isr2
    db 'isr2', 0

    extern isr20
    dq isr20
    db 'isr20', 0

    extern isr200
    dq isr200
    db 'isr200', 0

    extern isr201
    dq isr201
    db 'isr201', 0

    extern isr202
    dq isr202
    db 'isr202', 0

    extern isr203
    dq isr203
    db 'isr203', 0

    extern isr204
    dq isr204
    db 'isr204', 0

    extern isr205
    dq isr205
    db 'isr205', 0

    extern isr206
    dq isr206
    db 'isr206', 0

    extern isr207
    dq isr207
    db 'isr207', 0

    extern isr208
    dq isr208
    db 'isr208', 0

    extern isr209
    dq isr209
    db 'isr209', 0

    extern isr21
    dq isr21
    db 'isr21', 0

    extern isr210
    dq isr210
    db 'isr210', 0

    extern isr211
    dq isr211
    db 'isr211', 0

    extern isr212
    dq isr212
    db 'isr212', 0

    extern isr213
    dq isr213
    db 'isr213', 0

    extern isr214
    dq isr214
    db 'isr214', 0

    extern isr215
    dq isr215
    db 'isr215', 0

    extern isr216
    dq isr216
    db 'isr216', 0

    extern isr217
    dq isr217
    db 'isr217', 0

    extern isr218
    dq isr218
    db 'isr218', 0

    extern isr219
    dq isr219
    db 'isr219', 0

    extern isr22
    dq isr22
    db 'isr22', 0

    extern isr220
    dq isr220
    db 'isr220', 0

    extern isr221
    dq isr221
    db 'isr221', 0

    extern isr222
    dq isr222
    db 'isr222', 0

    extern isr223
    dq isr223
    db 'isr223', 0

    extern isr224
    dq isr224
    db 'isr224', 0

    extern isr225
    dq isr225
    db 'isr225', 0

    extern isr226
    dq isr226
    db 'isr226', 0

    extern isr227
    dq isr227
    db 'isr227', 0

    extern isr228
    dq isr228
    db 'isr228', 0

    extern isr229
    dq isr229
    db 'isr229', 0

    extern isr23
    dq isr23
    db 'isr23', 0

    extern isr230
    dq isr230
    db 'isr230', 0

    extern isr231
    dq isr231
    db 'isr231', 0

    extern isr232
    dq isr232
    db 'isr232', 0

    extern isr233
    dq isr233
    db 'isr233', 0

    extern isr234
    dq isr234
    db 'isr234', 0

    extern isr235
    dq isr235
    db 'isr235', 0

    extern isr236
    dq isr236
    db 'isr236', 0

    extern isr237
    dq isr237
    db 'isr237', 0

    extern isr238
    dq isr238
    db 'isr238', 0

    extern isr239
    dq isr239
    db 'isr239', 0

    extern isr24
    dq isr24
    db 'isr24', 0

    extern isr240
    dq isr240
    db 'isr240', 0

    extern isr241
    dq isr241
    db 'isr241', 0

    extern isr242
    dq isr242
    db 'isr242', 0

    extern isr243
    dq isr243
    db 'isr243', 0

    extern isr244
    dq isr244
    db 'isr244', 0

    extern isr245
    dq isr245
    db 'isr245', 0

    extern isr246
    dq isr246
    db 'isr246', 0

    extern isr247
    dq isr247
    db 'isr247', 0

    extern isr248
    dq isr248
    db 'isr248', 0

    extern isr249
    dq isr249
    db 'isr249', 0

    extern isr25
    dq isr25
    db 'isr25', 0

    extern isr250
    dq isr250
    db 'isr250', 0

    extern isr251
    dq isr251
    db 'isr251', 0

    extern isr252
    dq isr252
    db 'isr252', 0

    extern isr253
    dq isr253
    db 'isr253', 0

    extern isr254
    dq isr254
    db 'isr254', 0

    extern isr255
    dq isr255
    db 'isr255', 0

    extern isr26
    dq isr26
    db 'isr26', 0

    extern isr27
    dq isr27
    db 'isr27', 0

    extern isr28
    dq isr28
    db 'isr28', 0

    extern isr29
    dq isr29
    db 'isr29', 0

    extern isr3
    dq isr3
    db 'isr3', 0

    extern isr30
    dq isr30
    db 'isr30', 0

    extern isr31
    dq isr31
    db 'isr31', 0

    extern isr32
    dq isr32
    db 'isr32', 0

    extern isr33
    dq isr33
    db 'isr33', 0

    extern isr34
    dq isr34
    db 'isr34', 0

    extern isr35
    dq isr35
    db 'isr35', 0

    extern isr36
    dq isr36
    db 'isr36', 0

    extern isr37
    dq isr37
    db 'isr37', 0

    extern isr38
    dq isr38
    db 'isr38', 0

    extern isr39
    dq isr39
    db 'isr39', 0

    extern isr4
    dq isr4
    db 'isr4', 0

    extern isr40
    dq isr40
    db 'isr40', 0

    extern isr41
    dq isr41
    db 'isr41', 0

    extern isr42
    dq isr42
    db 'isr42', 0

    extern isr43
    dq isr43
    db 'isr43', 0

    extern isr44
    dq isr44
    db 'isr44', 0

    extern isr45
    dq isr45
    db 'isr45', 0

    extern isr46
    dq isr46
    db 'isr46', 0

    extern isr47
    dq isr47
    db 'isr47', 0

    extern isr48
    dq isr48
    db 'isr48', 0

    extern isr49
    dq isr49
    db 'isr49', 0

    extern isr5
    dq isr5
    db 'isr5', 0

    extern isr50
    dq isr50
    db 'isr50', 0

    extern isr51
    dq isr51
    db 'isr51', 0

    extern isr52
    dq isr52
    db 'isr52', 0

    extern isr53
    dq isr53
    db 'isr53', 0

    extern isr54
    dq isr54
    db 'isr54', 0

    extern isr55
    dq isr55
    db 'isr55', 0

    extern isr56
    dq isr56
    db 'isr56', 0

    extern isr57
    dq isr57
    db 'isr57', 0

    extern isr58
    dq isr58
    db 'isr58', 0

    extern isr59
    dq isr59
    db 'isr59', 0

    extern isr6
    dq isr6
    db 'isr6', 0

    extern isr60
    dq isr60
    db 'isr60', 0

    extern isr61
    dq isr61
    db 'isr61', 0

    extern isr62
    dq isr62
    db 'isr62', 0

    extern isr63
    dq isr63
    db 'isr63', 0

    extern isr64
    dq isr64
    db 'isr64', 0

    extern isr65
    dq isr65
    db 'isr65', 0

    extern isr66
    dq isr66
    db 'isr66', 0

    extern isr67
    dq isr67
    db 'isr67', 0

    extern isr68
    dq isr68
    db 'isr68', 0

    extern isr69
    dq isr69
    db 'isr69', 0

    extern isr7
    dq isr7
    db 'isr7', 0

    extern isr70
    dq isr70
    db 'isr70', 0

    extern isr71
    dq isr71
    db 'isr71', 0

    extern isr72
    dq isr72
    db 'isr72', 0

    extern isr73
    dq isr73
    db 'isr73', 0

    extern isr74
    dq isr74
    db 'isr74', 0

    extern isr75
    dq isr75
    db 'isr75', 0

    extern isr76
    dq isr76
    db 'isr76', 0

    extern isr77
    dq isr77
    db 'isr77', 0

    extern isr78
    dq isr78
    db 'isr78', 0

    extern isr79
    dq isr79
    db 'isr79', 0

    extern isr8
    dq isr8
    db 'isr8', 0

    extern isr80
    dq isr80
    db 'isr80', 0

    extern isr81
    dq isr81
    db 'isr81', 0

    extern isr82
    dq isr82
    db 'isr82', 0

    extern isr83
    dq isr83
    db 'isr83', 0

    extern isr84
    dq isr84
    db 'isr84', 0

    extern isr85
    dq isr85
    db 'isr85', 0

    extern isr86
    dq isr86
    db 'isr86', 0

    extern isr87
    dq isr87
    db 'isr87', 0

    extern isr88
    dq isr88
    db 'isr88', 0

    extern isr89
    dq isr89
    db 'isr89', 0

    extern isr9
    dq isr9
    db 'isr9', 0

    extern isr90
    dq isr90
    db 'isr90', 0

    extern isr91
    dq isr91
    db 'isr91', 0

    extern isr92
    dq isr92
    db 'isr92', 0

    extern isr93
    dq isr93
    db 'isr93', 0

    extern isr94
    dq isr94
    db 'isr94', 0

    extern isr95
    dq isr95
    db 'isr95', 0

    extern isr96
    dq isr96
    db 'isr96', 0

    extern isr97
    dq isr97
    db 'isr97', 0

    extern isr98
    dq isr98
    db 'isr98', 0

    extern isr99
    dq isr99
    db 'isr99', 0

    extern kcalloc
    dq kcalloc
    db 'kcalloc', 0

    extern kentry
    dq kentry
    db 'kentry', 0

    extern kentry_threaded
    dq kentry_threaded
    db 'kentry_threaded', 0

    extern kernel_address_req
    dq kernel_address_req
    db 'kernel_address_req', 0

    extern kfree
    dq kfree
    db 'kfree', 0

    extern kmalloc
    dq kmalloc
    db 'kmalloc', 0

    extern kopen
    dq kopen
    db 'kopen', 0

    extern krealloc
    dq krealloc
    db 'krealloc', 0

    extern ksleep
    dq ksleep
    db 'ksleep', 0

    extern lai_acpi_reset
    dq lai_acpi_reset
    db 'lai_acpi_reset', 0

    extern lai_amlname_done
    dq lai_amlname_done
    db 'lai_amlname_done', 0

    extern lai_amlname_iterate
    dq lai_amlname_iterate
    db 'lai_amlname_iterate', 0

    extern lai_amlname_parse
    dq lai_amlname_parse
    db 'lai_amlname_parse', 0

    extern lai_api_error_to_string
    dq lai_api_error_to_string
    db 'lai_api_error_to_string', 0

    extern lai_calloc
    dq lai_calloc
    db 'lai_calloc', 0

    extern lai_check_device_pnp_id
    dq lai_check_device_pnp_id
    db 'lai_check_device_pnp_id', 0

    extern lai_create_buffer
    dq lai_create_buffer
    db 'lai_create_buffer', 0

    extern lai_create_c_string
    dq lai_create_c_string
    db 'lai_create_c_string', 0

    extern lai_create_namespace
    dq lai_create_namespace
    db 'lai_create_namespace', 0

    extern lai_create_nsnode
    dq lai_create_nsnode
    db 'lai_create_nsnode', 0

    extern lai_create_nsnode_or_die
    dq lai_create_nsnode_or_die
    db 'lai_create_nsnode_or_die', 0

    extern lai_create_pkg
    dq lai_create_pkg
    db 'lai_create_pkg', 0

    extern lai_create_root
    dq lai_create_root
    db 'lai_create_root', 0

    extern lai_create_string
    dq lai_create_string
    db 'lai_create_string', 0

    extern lai_current_instance
    dq lai_current_instance
    db 'lai_current_instance', 0

    extern lai_debug
    dq lai_debug
    db 'lai_debug', 0

    extern lai_disable_acpi
    dq lai_disable_acpi
    db 'lai_disable_acpi', 0

    extern lai_do_os_method
    dq lai_do_os_method
    db 'lai_do_os_method', 0

    extern lai_do_osi_method
    dq lai_do_osi_method
    db 'lai_do_osi_method', 0

    extern lai_do_resolve
    dq lai_do_resolve
    db 'lai_do_resolve', 0

    extern lai_do_resolve_new_node
    dq lai_do_resolve_new_node
    db 'lai_do_resolve_new_node', 0

    extern lai_do_rev_method
    dq lai_do_rev_method
    db 'lai_do_rev_method', 0

    extern lai_eisaid
    dq lai_eisaid
    db 'lai_eisaid', 0

    extern lai_enable_acpi
    dq lai_enable_acpi
    db 'lai_enable_acpi', 0

    extern lai_enable_tracing
    dq lai_enable_tracing
    db 'lai_enable_tracing', 0

    extern lai_enter_sleep
    dq lai_enter_sleep
    db 'lai_enter_sleep', 0

    extern lai_eval
    dq lai_eval
    db 'lai_eval', 0

    extern lai_eval_args
    dq lai_eval_args
    db 'lai_eval_args', 0

    extern lai_eval_largs
    dq lai_eval_largs
    db 'lai_eval_largs', 0

    extern lai_eval_vargs
    dq lai_eval_vargs
    db 'lai_eval_vargs', 0

    extern lai_evaluate_sta
    dq lai_evaluate_sta
    db 'lai_evaluate_sta', 0

    extern lai_exec_access
    dq lai_exec_access
    db 'lai_exec_access', 0

    extern lai_exec_get_integer
    dq lai_exec_get_integer
    db 'lai_exec_get_integer', 0

    extern lai_exec_get_objectref
    dq lai_exec_get_objectref
    db 'lai_exec_get_objectref', 0

    extern lai_exec_mutate_ns
    dq lai_exec_mutate_ns
    db 'lai_exec_mutate_ns', 0

    extern lai_exec_pkg_var_load
    dq lai_exec_pkg_var_load
    db 'lai_exec_pkg_var_load', 0

    extern lai_exec_pkg_var_store
    dq lai_exec_pkg_var_store
    db 'lai_exec_pkg_var_store', 0

    extern lai_exec_ref_load
    dq lai_exec_ref_load
    db 'lai_exec_ref_load', 0

    extern lai_exec_ref_store
    dq lai_exec_ref_store
    db 'lai_exec_ref_store', 0

    extern lai_exec_string_length
    dq lai_exec_string_length
    db 'lai_exec_string_length', 0

    extern lai_finalize_state
    dq lai_finalize_state
    db 'lai_finalize_state', 0

    extern lai_get_sci_event
    dq lai_get_sci_event
    db 'lai_get_sci_event', 0

    extern lai_init_children
    dq lai_init_children
    db 'lai_init_children', 0

    extern lai_init_state
    dq lai_init_state
    db 'lai_init_state', 0

    extern lai_install_nsnode
    dq lai_install_nsnode
    db 'lai_install_nsnode', 0

    extern lai_is_name
    dq lai_is_name
    db 'lai_is_name', 0

    extern lai_mutate_buffer
    dq lai_mutate_buffer
    db 'lai_mutate_buffer', 0

    extern lai_mutate_integer
    dq lai_mutate_integer
    db 'lai_mutate_integer', 0

    extern lai_mutate_string
    dq lai_mutate_string
    db 'lai_mutate_string', 0

    extern lai_ns_child_iterate
    dq lai_ns_child_iterate
    db 'lai_ns_child_iterate', 0

    extern lai_ns_get_child
    dq lai_ns_get_child
    db 'lai_ns_get_child', 0

    extern lai_ns_get_node_type
    dq lai_ns_get_node_type
    db 'lai_ns_get_node_type', 0

    extern lai_ns_get_opregion_address_space
    dq lai_ns_get_opregion_address_space
    db 'lai_ns_get_opregion_address_space', 0

    extern lai_ns_get_parent
    dq lai_ns_get_parent
    db 'lai_ns_get_parent', 0

    extern lai_ns_get_root
    dq lai_ns_get_root
    db 'lai_ns_get_root', 0

    extern lai_ns_iterate
    dq lai_ns_iterate
    db 'lai_ns_iterate', 0

    extern lai_ns_override_notify
    dq lai_ns_override_notify
    db 'lai_ns_override_notify', 0

    extern lai_ns_override_opregion
    dq lai_ns_override_opregion
    db 'lai_ns_override_opregion', 0

    extern lai_obj_clone
    dq lai_obj_clone
    db 'lai_obj_clone', 0

    extern lai_obj_exec_match_op
    dq lai_obj_exec_match_op
    db 'lai_obj_exec_match_op', 0

    extern lai_obj_get_handle
    dq lai_obj_get_handle
    db 'lai_obj_get_handle', 0

    extern lai_obj_get_integer
    dq lai_obj_get_integer
    db 'lai_obj_get_integer', 0

    extern lai_obj_get_pkg
    dq lai_obj_get_pkg
    db 'lai_obj_get_pkg', 0

    extern lai_obj_get_type
    dq lai_obj_get_type
    db 'lai_obj_get_type', 0

    extern lai_obj_resize_buffer
    dq lai_obj_resize_buffer
    db 'lai_obj_resize_buffer', 0

    extern lai_obj_resize_pkg
    dq lai_obj_resize_pkg
    db 'lai_obj_resize_pkg', 0

    extern lai_obj_resize_string
    dq lai_obj_resize_string
    db 'lai_obj_resize_string', 0

    extern lai_obj_to_buffer
    dq lai_obj_to_buffer
    db 'lai_obj_to_buffer', 0

    extern lai_obj_to_decimal_string
    dq lai_obj_to_decimal_string
    db 'lai_obj_to_decimal_string', 0

    extern lai_obj_to_hex_string
    dq lai_obj_to_hex_string
    db 'lai_obj_to_hex_string', 0

    extern lai_obj_to_integer
    dq lai_obj_to_integer
    db 'lai_obj_to_integer', 0

    extern lai_obj_to_string
    dq lai_obj_to_string
    db 'lai_obj_to_string', 0

    extern lai_obj_to_type_string
    dq lai_obj_to_type_string
    db 'lai_obj_to_type_string', 0

    extern lai_objecttype_ns
    dq lai_objecttype_ns
    db 'lai_objecttype_ns', 0

    extern lai_objecttype_obj
    dq lai_objecttype_obj
    db 'lai_objecttype_obj', 0

    extern lai_operand_emplace
    dq lai_operand_emplace
    db 'lai_operand_emplace', 0

    extern lai_operand_load
    dq lai_operand_load
    db 'lai_operand_load', 0

    extern lai_operand_mutate
    dq lai_operand_mutate
    db 'lai_operand_mutate', 0

    extern lai_panic
    dq lai_panic
    db 'lai_panic', 0

    extern lai_pci_find_bus
    dq lai_pci_find_bus
    db 'lai_pci_find_bus', 0

    extern lai_pci_find_device
    dq lai_pci_find_device
    db 'lai_pci_find_device', 0

    extern lai_pci_parse_prt
    dq lai_pci_parse_prt
    db 'lai_pci_parse_prt', 0

    extern lai_pci_route
    dq lai_pci_route
    db 'lai_pci_route', 0

    extern lai_pci_route_pin
    dq lai_pci_route_pin
    db 'lai_pci_route_pin', 0

    extern lai_populate
    dq lai_populate
    db 'lai_populate', 0

    extern lai_read_bankfield
    dq lai_read_bankfield
    db 'lai_read_bankfield', 0

    extern lai_read_field
    dq lai_read_field
    db 'lai_read_field', 0

    extern lai_read_field_internal
    dq lai_read_field_internal
    db 'lai_read_field_internal', 0

    extern lai_read_opregion
    dq lai_read_opregion
    db 'lai_read_opregion', 0

    extern lai_read_resource
    dq lai_read_resource
    db 'lai_read_resource', 0

    extern lai_resolve_new_node
    dq lai_resolve_new_node
    db 'lai_resolve_new_node', 0

    extern lai_resolve_path
    dq lai_resolve_path
    db 'lai_resolve_path', 0

    extern lai_resolve_search
    dq lai_resolve_search
    db 'lai_resolve_search', 0

    extern lai_resource_get_type
    dq lai_resource_get_type
    db 'lai_resource_get_type', 0

    extern lai_resource_irq_is_active_low
    dq lai_resource_irq_is_active_low
    db 'lai_resource_irq_is_active_low', 0

    extern lai_resource_irq_is_level_triggered
    dq lai_resource_irq_is_level_triggered
    db 'lai_resource_irq_is_level_triggered', 0

    extern lai_resource_iterate
    dq lai_resource_iterate
    db 'lai_resource_iterate', 0

    extern lai_resource_next_irq
    dq lai_resource_next_irq
    db 'lai_resource_next_irq', 0

    extern lai_set_acpi_revision
    dq lai_set_acpi_revision
    db 'lai_set_acpi_revision', 0

    extern lai_set_sci_event
    dq lai_set_sci_event
    db 'lai_set_sci_event', 0

    extern lai_snprintf
    dq lai_snprintf
    db 'lai_snprintf', 0

    extern lai_store_ns
    dq lai_store_ns
    db 'lai_store_ns', 0

    extern lai_strcmp
    dq lai_strcmp
    db 'lai_strcmp', 0

    extern lai_strcpy
    dq lai_strcpy
    db 'lai_strcpy', 0

    extern lai_stringify_amlname
    dq lai_stringify_amlname
    db 'lai_stringify_amlname', 0

    extern lai_stringify_node_path
    dq lai_stringify_node_path
    db 'lai_stringify_node_path', 0

    extern lai_strlen
    dq lai_strlen
    db 'lai_strlen', 0

    extern lai_swap_object
    dq lai_swap_object
    db 'lai_swap_object', 0

    extern lai_uninstall_nsnode
    dq lai_uninstall_nsnode
    db 'lai_uninstall_nsnode', 0

    extern lai_var_assign
    dq lai_var_assign
    db 'lai_var_assign', 0

    extern lai_var_finalize
    dq lai_var_finalize
    db 'lai_var_finalize', 0

    extern lai_var_move
    dq lai_var_move
    db 'lai_var_move', 0

    extern lai_vsnprintf
    dq lai_vsnprintf
    db 'lai_vsnprintf', 0

    extern lai_warn
    dq lai_warn
    db 'lai_warn', 0

    extern lai_write_bankfield
    dq lai_write_bankfield
    db 'lai_write_bankfield', 0

    extern lai_write_field
    dq lai_write_field
    db 'lai_write_field', 0

    extern lai_write_field_internal
    dq lai_write_field_internal
    db 'lai_write_field_internal', 0

    extern lai_write_opregion
    dq lai_write_opregion
    db 'lai_write_opregion', 0

    extern laihost_free
    dq laihost_free
    db 'laihost_free', 0

    extern laihost_inb
    dq laihost_inb
    db 'laihost_inb', 0

    extern laihost_ind
    dq laihost_ind
    db 'laihost_ind', 0

    extern laihost_inw
    dq laihost_inw
    db 'laihost_inw', 0

    extern laihost_log
    dq laihost_log
    db 'laihost_log', 0

    extern laihost_malloc
    dq laihost_malloc
    db 'laihost_malloc', 0

    extern laihost_map
    dq laihost_map
    db 'laihost_map', 0

    extern laihost_outb
    dq laihost_outb
    db 'laihost_outb', 0

    extern laihost_outd
    dq laihost_outd
    db 'laihost_outd', 0

    extern laihost_outw
    dq laihost_outw
    db 'laihost_outw', 0

    extern laihost_panic
    dq laihost_panic
    db 'laihost_panic', 0

    extern laihost_pci_readb
    dq laihost_pci_readb
    db 'laihost_pci_readb', 0

    extern laihost_pci_readd
    dq laihost_pci_readd
    db 'laihost_pci_readd', 0

    extern laihost_pci_readw
    dq laihost_pci_readw
    db 'laihost_pci_readw', 0

    extern laihost_pci_writeb
    dq laihost_pci_writeb
    db 'laihost_pci_writeb', 0

    extern laihost_pci_writed
    dq laihost_pci_writed
    db 'laihost_pci_writed', 0

    extern laihost_pci_writew
    dq laihost_pci_writew
    db 'laihost_pci_writew', 0

    extern laihost_realloc
    dq laihost_realloc
    db 'laihost_realloc', 0

    extern laihost_scan
    dq laihost_scan
    db 'laihost_scan', 0

    extern laihost_sleep
    dq laihost_sleep
    db 'laihost_sleep', 0

    extern laihost_timer
    dq laihost_timer
    db 'laihost_timer', 0

    extern laihost_unmap
    dq laihost_unmap
    db 'laihost_unmap', 0

    extern lapic_read
    dq lapic_read
    db 'lapic_read', 0

    extern lapic_write
    dq lapic_write
    db 'lapic_write', 0

    extern lfind
    dq lfind
    db 'lfind', 0

    extern limine_kernel_address_request
    dq limine_kernel_address_request
    db 'limine_kernel_address_request', 0

    extern limine_memmap_request
    dq limine_memmap_request
    db 'limine_memmap_request', 0

    extern limine_rsdp_request
    dq limine_rsdp_request
    db 'limine_rsdp_request', 0

    extern limine_stack_size_request
    dq limine_stack_size_request
    db 'limine_stack_size_request', 0

    extern list_contains
    dq list_contains
    db 'list_contains', 0

    extern list_create
    dq list_create
    db 'list_create', 0

    extern list_destroy
    dq list_destroy
    db 'list_destroy', 0

    extern list_destroy_node
    dq list_destroy_node
    db 'list_destroy_node', 0

    extern list_get_back
    dq list_get_back
    db 'list_get_back', 0

    extern list_get_front
    dq list_get_front
    db 'list_get_front', 0

    extern list_get_indexed
    dq list_get_indexed
    db 'list_get_indexed', 0

    extern list_pop
    dq list_pop
    db 'list_pop', 0

    extern list_push_back
    dq list_push_back
    db 'list_push_back', 0

    extern list_push_before
    dq list_push_before
    db 'list_push_before', 0

    extern list_push_front
    dq list_push_front
    db 'list_push_front', 0

    extern list_remove
    dq list_remove
    db 'list_remove', 0

    extern list_remove_back
    dq list_remove_back
    db 'list_remove_back', 0

    extern list_remove_front
    dq list_remove_front
    db 'list_remove_front', 0

    extern list_remove_indexed
    dq list_remove_indexed
    db 'list_remove_indexed', 0

    extern memcmp
    dq memcmp
    db 'memcmp', 0

    extern memcpy
    dq memcpy
    db 'memcpy', 0

    extern memcpy64
    dq memcpy64
    db 'memcpy64', 0

    extern memmap_req
    dq memmap_req
    db 'memmap_req', 0

    extern memmove
    dq memmove
    db 'memmove', 0

    extern memset
    dq memset
    db 'memset', 0

    extern module_load
    dq module_load
    db 'module_load', 0

    extern modules_init
    dq modules_init
    db 'modules_init', 0

    extern nvme_create_queue_pair
    dq nvme_create_queue_pair
    db 'nvme_create_queue_pair', 0

    extern nvme_execute_read_write
    dq nvme_execute_read_write
    db 'nvme_execute_read_write', 0

    extern nvme_get_ns_info
    dq nvme_get_ns_info
    db 'nvme_get_ns_info', 0

    extern nvme_identify
    dq nvme_identify
    db 'nvme_identify', 0

    extern nvme_init
    dq nvme_init
    db 'nvme_init', 0

    extern nvme_lseek
    dq nvme_lseek
    db 'nvme_lseek', 0

    extern nvme_read
    dq nvme_read
    db 'nvme_read', 0

    extern nvme_send_command
    dq nvme_send_command
    db 'nvme_send_command', 0

    extern nvme_send_command_and_wait
    dq nvme_send_command_and_wait
    db 'nvme_send_command_and_wait', 0

    extern nvme_set_queue_count
    dq nvme_set_queue_count
    db 'nvme_set_queue_count', 0

    extern nvme_write
    dq nvme_write
    db 'nvme_write', 0

    extern partfs_init
    dq partfs_init
    db 'partfs_init', 0

    extern partfs_lseek
    dq partfs_lseek
    db 'partfs_lseek', 0

    extern partfs_read
    dq partfs_read
    db 'partfs_read', 0

    extern partfs_write
    dq partfs_write
    db 'partfs_write', 0

    extern pciIrqPins
    dq pciIrqPins
    db 'pciIrqPins', 0

    extern pci_device_read_config
    dq pci_device_read_config
    db 'pci_device_read_config', 0

    extern pci_device_write_config
    dq pci_device_write_config
    db 'pci_device_write_config', 0

    extern pci_enable_busmastering
    dq pci_enable_busmastering
    db 'pci_enable_busmastering', 0

    extern pci_enable_mmio
    dq pci_enable_mmio
    db 'pci_enable_mmio', 0

    extern pci_get_device
    dq pci_get_device
    db 'pci_get_device', 0

    extern pci_get_device_by_vendor
    dq pci_get_device_by_vendor
    db 'pci_get_device_by_vendor', 0

    extern pci_init
    dq pci_init
    db 'pci_init', 0

    extern pci_preinit
    dq pci_preinit
    db 'pci_preinit', 0

    extern pci_read_bar
    dq pci_read_bar
    db 'pci_read_bar', 0

    extern pci_read_config
    dq pci_read_config
    db 'pci_read_config', 0

    extern pci_write_config
    dq pci_write_config
    db 'pci_write_config', 0

    extern pipe_close
    dq pipe_close
    db 'pipe_close', 0

    extern pipe_create
    dq pipe_create
    db 'pipe_create', 0

    extern pipe_read
    dq pipe_read
    db 'pipe_read', 0

    extern pipe_write
    dq pipe_write
    db 'pipe_write', 0

    extern pmm_alloc
    dq pmm_alloc
    db 'pmm_alloc', 0

    extern pmm_alloc_advanced
    dq pmm_alloc_advanced
    db 'pmm_alloc_advanced', 0

    extern pmm_free
    dq pmm_free
    db 'pmm_free', 0

    extern pmm_init
    dq pmm_init
    db 'pmm_init', 0

    extern printf
    dq printf
    db 'printf', 0

    extern printf_scheduler
    dq printf_scheduler
    db 'printf_scheduler', 0

    extern printf_write_char_e8
    dq printf_write_char_e8
    db 'printf_write_char_e8', 0

    extern printf_write_char_str
    dq printf_write_char_str
    db 'printf_write_char_str', 0

    extern process_create
    dq process_create
    db 'process_create', 0

    extern qemu_debug_puts_urgent
    dq qemu_debug_puts_urgent
    db 'qemu_debug_puts_urgent', 0

    extern rsdp_req
    dq rsdp_req
    db 'rsdp_req', 0

    extern scheduler_add_task
    dq scheduler_add_task
    db 'scheduler_add_task', 0

    extern scheduler_get_current_pid
    dq scheduler_get_current_pid
    db 'scheduler_get_current_pid', 0

    extern scheduler_init
    dq scheduler_init
    db 'scheduler_init', 0

    extern scheduler_schedule
    dq scheduler_schedule
    db 'scheduler_schedule', 0

    extern scheduler_schedule_smp
    dq scheduler_schedule_smp
    db 'scheduler_schedule_smp', 0

    extern scheduler_switch_task
    dq scheduler_switch_task
    db 'scheduler_switch_task', 0

    extern smp_init
    dq smp_init
    db 'smp_init', 0

    extern smp_init_cpu_misc
    dq smp_init_cpu_misc
    db 'smp_init_cpu_misc', 0

    extern smp_init_trampoline
    dq smp_init_trampoline
    db 'smp_init_trampoline', 0

    extern sprintf
    dq sprintf
    db 'sprintf', 0

    extern stack_size_req
    dq stack_size_req
    db 'stack_size_req', 0

    extern strchr
    dq strchr
    db 'strchr', 0

    extern strchrnul
    dq strchrnul
    db 'strchrnul', 0

    extern strcmp
    dq strcmp
    db 'strcmp', 0

    extern strcpy
    dq strcpy
    db 'strcpy', 0

    extern strcspn
    dq strcspn
    db 'strcspn', 0

    extern strdup
    dq strdup
    db 'strdup', 0

    extern strlen
    dq strlen
    db 'strlen', 0

    extern strncmp
    dq strncmp
    db 'strncmp', 0

    extern strpbrk
    dq strpbrk
    db 'strpbrk', 0

    extern strsep
    dq strsep
    db 'strsep', 0

    extern strspn
    dq strspn
    db 'strspn', 0

    extern strstr
    dq strstr
    db 'strstr', 0

    extern strtok_r
    dq strtok_r
    db 'strtok_r', 0

    extern syscall_alloc
    dq syscall_alloc
    db 'syscall_alloc', 0

    extern syscall_close
    dq syscall_close
    db 'syscall_close', 0

    extern syscall_debug
    dq syscall_debug
    db 'syscall_debug', 0

    extern syscall_entry
    dq syscall_entry
    db 'syscall_entry', 0

    extern syscall_fork
    dq syscall_fork
    db 'syscall_fork', 0

    extern syscall_install
    dq syscall_install
    db 'syscall_install', 0

    extern syscall_ioctl
    dq syscall_ioctl
    db 'syscall_ioctl', 0

    extern syscall_module_or_invalid
    dq syscall_module_or_invalid
    db 'syscall_module_or_invalid', 0

    extern syscall_open
    dq syscall_open
    db 'syscall_open', 0

    extern syscall_privilege_check
    dq syscall_privilege_check
    db 'syscall_privilege_check', 0

    extern syscall_read
    dq syscall_read
    db 'syscall_read', 0

    extern syscall_seek
    dq syscall_seek
    db 'syscall_seek', 0

    extern syscall_tcb_set
    dq syscall_tcb_set
    db 'syscall_tcb_set', 0

    extern syscall_write
    dq syscall_write
    db 'syscall_write', 0

    extern thread_create
    dq thread_create
    db 'thread_create', 0

    extern timer_get_nanoseconds
    dq timer_get_nanoseconds
    db 'timer_get_nanoseconds', 0

    extern timer_handler
    dq timer_handler
    db 'timer_handler', 0

    extern timer_init
    dq timer_init
    db 'timer_init', 0

    extern vfs_close
    dq vfs_close
    db 'vfs_close', 0

    extern vfs_create
    dq vfs_create
    db 'vfs_create', 0

    extern vfs_finddir
    dq vfs_finddir
    db 'vfs_finddir', 0

    extern vfs_get_mountpoint
    dq vfs_get_mountpoint
    db 'vfs_get_mountpoint', 0

    extern vfs_init
    dq vfs_init
    db 'vfs_init', 0

    extern vfs_ioctl
    dq vfs_ioctl
    db 'vfs_ioctl', 0

    extern vfs_lseek
    dq vfs_lseek
    db 'vfs_lseek', 0

    extern vfs_mount
    dq vfs_mount
    db 'vfs_mount', 0

    extern vfs_node_mount
    dq vfs_node_mount
    db 'vfs_node_mount', 0

    extern vfs_open
    dq vfs_open
    db 'vfs_open', 0

    extern vfs_print_tree
    dq vfs_print_tree
    db 'vfs_print_tree', 0

    extern vfs_read
    dq vfs_read
    db 'vfs_read', 0

    extern vfs_write
    dq vfs_write
    db 'vfs_write', 0

    extern vmm_copy_pml4
    dq vmm_copy_pml4
    db 'vmm_copy_pml4', 0

    extern vmm_free_pml4
    dq vmm_free_pml4
    db 'vmm_free_pml4', 0

    extern vmm_get_phys
    dq vmm_get_phys
    db 'vmm_get_phys', 0

    extern vmm_init
    dq vmm_init
    db 'vmm_init', 0

    extern vmm_map
    dq vmm_map
    db 'vmm_map', 0

    extern vmm_map_huge
    dq vmm_map_huge
    db 'vmm_map_huge', 0

    extern vmm_map_if_not_mapped
    dq vmm_map_if_not_mapped
    db 'vmm_map_if_not_mapped', 0

    extern vmm_read
    dq vmm_read
    db 'vmm_read', 0

    extern vmm_setup_pml4
    dq vmm_setup_pml4
    db 'vmm_setup_pml4', 0

    extern vmm_unmap
    dq vmm_unmap
    db 'vmm_unmap', 0

    extern yield
    dq yield
    db 'yield', 0
global kernel_symbols_end
kernel_symbols_end: ;
