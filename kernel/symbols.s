SECTION .symbols
global kernel_symbols_start
kernel_symbols_start:

    extern __int_str
    dq __int_str
    db '__int_str', 0

    extern _vsnprintf
    dq _vsnprintf
    db '_vsnprintf', 0

    extern acpi_find_sdt
    dq acpi_find_sdt
    db 'acpi_find_sdt', 0

    extern acpi_init
    dq acpi_init
    db 'acpi_init', 0

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

    extern cpuLocals
    dq cpuLocals
    db 'cpuLocals', 0

    extern create_new_pml4
    dq create_new_pml4
    db 'create_new_pml4', 0

    extern debug_out_char
    dq debug_out_char
    db 'debug_out_char', 0

    extern dispatch_interrupt
    dq dispatch_interrupt
    db 'dispatch_interrupt', 0

    extern displayCharacter
    dq displayCharacter
    db 'displayCharacter', 0

    extern displayString
    dq displayString
    db 'displayString', 0

    extern g_bootstrap_stack
    dq g_bootstrap_stack
    db 'g_bootstrap_stack', 0

    extern g_header_tag_framebuffer
    dq g_header_tag_framebuffer
    db 'g_header_tag_framebuffer', 0

    extern gdt_init
    dq gdt_init
    db 'gdt_init', 0

    extern gdt_load_tss
    dq gdt_load_tss
    db 'gdt_load_tss', 0

    extern get_mountpoint
    dq get_mountpoint
    db 'get_mountpoint', 0

    extern get_mountpoint_recur
    dq get_mountpoint_recur
    db 'get_mountpoint_recur', 0

    extern get_next_task
    dq get_next_task
    db 'get_next_task', 0

    extern global_pmm_lock
    dq global_pmm_lock
    db 'global_pmm_lock', 0

    extern handlers
    dq handlers
    db 'handlers', 0

    extern hashmap_create
    dq hashmap_create
    db 'hashmap_create', 0

    extern hashmap_free
    dq hashmap_free
    db 'hashmap_free', 0

    extern hashmap_get
    dq hashmap_get
    db 'hashmap_get', 0

    extern hashmap_has
    dq hashmap_has
    db 'hashmap_has', 0

    extern hashmap_keys
    dq hashmap_keys
    db 'hashmap_keys', 0

    extern hashmap_remove
    dq hashmap_remove
    db 'hashmap_remove', 0

    extern hashmap_set
    dq hashmap_set
    db 'hashmap_set', 0

    extern hashmap_string_comp
    dq hashmap_string_comp
    db 'hashmap_string_comp', 0

    extern hashmap_string_dupe
    dq hashmap_string_dupe
    db 'hashmap_string_dupe', 0

    extern hashmap_string_hash
    dq hashmap_string_hash
    db 'hashmap_string_hash', 0

    extern hashmap_values
    dq hashmap_values
    db 'hashmap_values', 0

    extern header
    dq header
    db 'header', 0

    extern idt_init
    dq idt_init
    db 'idt_init', 0

    extern inb
    dq inb
    db 'inb', 0

    extern initrd_finddir
    dq initrd_finddir
    db 'initrd_finddir', 0

    extern initrd_fstat
    dq initrd_fstat
    db 'initrd_fstat', 0

    extern initrd_lseek
    dq initrd_lseek
    db 'initrd_lseek', 0

    extern initrd_open
    dq initrd_open
    db 'initrd_open', 0

    extern initrd_read
    dq initrd_read
    db 'initrd_read', 0

    extern initrd_write
    dq initrd_write
    db 'initrd_write', 0

    extern inl
    dq inl
    db 'inl', 0

    extern inw
    dq inw
    db 'inw', 0

    extern ioapics
    dq ioapics
    db 'ioapics', 0

    extern ipi_resched
    dq ipi_resched
    db 'ipi_resched', 0

    extern irq_functions
    dq irq_functions
    db 'irq_functions', 0

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

    extern kernel_pml4
    dq kernel_pml4
    db 'kernel_pml4', 0

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

    extern lai_read_field
    dq lai_read_field
    db 'lai_read_field', 0

    extern lai_read_field_internal
    dq lai_read_field_internal
    db 'lai_read_field_internal', 0

    extern lai_read_indexfield
    dq lai_read_indexfield
    db 'lai_read_indexfield', 0

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

    extern lai_write_field
    dq lai_write_field
    db 'lai_write_field', 0

    extern lai_write_field_internal
    dq lai_write_field_internal
    db 'lai_write_field_internal', 0

    extern lai_write_indexfield
    dq lai_write_indexfield
    db 'lai_write_indexfield', 0

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

    extern laihost_unmap
    dq laihost_unmap
    db 'laihost_unmap', 0

    extern lapic_enable
    dq lapic_enable
    db 'lapic_enable', 0

    extern lapic_init
    dq lapic_init
    db 'lapic_init', 0

    extern lapic_legacy_irq
    dq lapic_legacy_irq
    db 'lapic_legacy_irq', 0

    extern lapic_read
    dq lapic_read
    db 'lapic_read', 0

    extern lapic_redirect
    dq lapic_redirect
    db 'lapic_redirect', 0

    extern lapic_write
    dq lapic_write
    db 'lapic_write', 0

    extern lgdt
    dq lgdt
    db 'lgdt', 0

    extern lidt
    dq lidt
    db 'lidt', 0

    extern list2str
    dq list2str
    db 'list2str', 0

    extern list_contain
    dq list_contain
    db 'list_contain', 0

    extern list_create
    dq list_create
    db 'list_create', 0

    extern list_destroy
    dq list_destroy
    db 'list_destroy', 0

    extern list_get_back
    dq list_get_back
    db 'list_get_back', 0

    extern list_get_front
    dq list_get_front
    db 'list_get_front', 0

    extern list_get_node_by_index
    dq list_get_node_by_index
    db 'list_get_node_by_index', 0

    extern list_insert_back
    dq list_insert_back
    db 'list_insert_back', 0

    extern list_insert_front
    dq list_insert_front
    db 'list_insert_front', 0

    extern list_pop
    dq list_pop
    db 'list_pop', 0

    extern list_remove_back
    dq list_remove_back
    db 'list_remove_back', 0

    extern list_remove_front
    dq list_remove_front
    db 'list_remove_front', 0

    extern list_remove_index
    dq list_remove_index
    db 'list_remove_index', 0

    extern list_remove_node
    dq list_remove_node
    db 'list_remove_node', 0

    extern list_size
    dq list_size
    db 'list_size', 0

    extern lnode_destroy
    dq lnode_destroy
    db 'lnode_destroy', 0

    extern madt
    dq madt
    db 'madt', 0

    extern memcmp
    dq memcmp
    db 'memcmp', 0

    extern memcpy
    dq memcpy
    db 'memcpy', 0

    extern memset
    dq memset
    db 'memset', 0

    extern module_load
    dq module_load
    db 'module_load', 0

    extern module_unload
    dq module_unload
    db 'module_unload', 0

    extern modules_get_list
    dq modules_get_list
    db 'modules_get_list', 0

    extern modules_get_symbols
    dq modules_get_symbols
    db 'modules_get_symbols', 0

    extern modules_init
    dq modules_init
    db 'modules_init', 0

    extern outb
    dq outb
    db 'outb', 0

    extern outl
    dq outl
    db 'outl', 0

    extern outw
    dq outw
    db 'outw', 0

    extern parseTarInitrd
    dq parseTarInitrd
    db 'parseTarInitrd', 0

    extern pci_init
    dq pci_init
    db 'pci_init', 0

    extern pci_read_byte
    dq pci_read_byte
    db 'pci_read_byte', 0

    extern pci_read_dword
    dq pci_read_dword
    db 'pci_read_dword', 0

    extern pci_read_word
    dq pci_read_word
    db 'pci_read_word', 0

    extern pci_write_byte
    dq pci_write_byte
    db 'pci_write_byte', 0

    extern pci_write_dword
    dq pci_write_dword
    db 'pci_write_dword', 0

    extern pci_write_word
    dq pci_write_word
    db 'pci_write_word', 0

    extern pit_handler
    dq pit_handler
    db 'pit_handler', 0

    extern pmmBitmap
    dq pmmBitmap
    db 'pmmBitmap', 0

    extern pmmFreePages
    dq pmmFreePages
    db 'pmmFreePages', 0

    extern pmmLength
    dq pmmLength
    db 'pmmLength', 0

    extern pmmTotalPages
    dq pmmTotalPages
    db 'pmmTotalPages', 0

    extern pmm_alloc
    dq pmm_alloc
    db 'pmm_alloc', 0

    extern pmm_alloc_advanced
    dq pmm_alloc_advanced
    db 'pmm_alloc_advanced', 0

    extern pmm_alloc_nonzero
    dq pmm_alloc_nonzero
    db 'pmm_alloc_nonzero', 0

    extern pmm_free
    dq pmm_free
    db 'pmm_free', 0

    extern pmm_init
    dq pmm_init
    db 'pmm_init', 0

    extern print_vfstree
    dq print_vfstree
    db 'print_vfstree', 0

    extern printf
    dq printf
    db 'printf', 0

    extern processors
    dq processors
    db 'processors', 0

    extern processors_count
    dq processors_count
    db 'processors_count', 0

    extern sched_ready
    dq sched_ready
    db 'sched_ready', 0

    extern schedule
    dq schedule
    db 'schedule', 0

    extern scheduler_init
    dq scheduler_init
    db 'scheduler_init', 0

    extern service_interrupt2
    dq service_interrupt2
    db 'service_interrupt2', 0

    extern service_interrupt3
    dq service_interrupt3
    db 'service_interrupt3', 0

    extern smp_check_ap_flag
    dq smp_check_ap_flag
    db 'smp_check_ap_flag', 0

    extern smp_init
    dq smp_init
    db 'smp_init', 0

    extern smp_init_cpu0_local
    dq smp_init_cpu0_local
    db 'smp_init_cpu0_local', 0

    extern smp_init_trampoline
    dq smp_init_trampoline
    db 'smp_init_trampoline', 0

    extern special_thing
    dq special_thing
    db 'special_thing', 0

    extern sprintf
    dq sprintf
    db 'sprintf', 0

    extern startswith
    dq startswith
    db 'startswith', 0

    extern strcat
    dq strcat
    db 'strcat', 0

    extern strcmp
    dq strcmp
    db 'strcmp', 0

    extern strcpy
    dq strcpy
    db 'strcpy', 0

    extern strdup
    dq strdup
    db 'strdup', 0

    extern strlen
    dq strlen
    db 'strlen', 0

    extern strncmp
    dq strncmp
    db 'strncmp', 0

    extern strsep
    dq strsep
    db 'strsep', 0

    extern strstr
    dq strstr
    db 'strstr', 0

    extern strtok
    dq strtok
    db 'strtok', 0

    extern strtoklist
    dq strtoklist
    db 'strtoklist', 0

    extern task_return_context
    dq task_return_context
    db 'task_return_context', 0

    extern thread_create
    dq thread_create
    db 'thread_create', 0

    extern thread_main
    dq thread_main
    db 'thread_main', 0

    extern tree2array
    dq tree2array
    db 'tree2array', 0

    extern tree2array_recur
    dq tree2array_recur
    db 'tree2array_recur', 0

    extern tree2list
    dq tree2list
    db 'tree2list', 0

    extern tree2list_recur
    dq tree2list_recur
    db 'tree2list_recur', 0

    extern tree_create
    dq tree_create
    db 'tree_create', 0

    extern tree_find_parent
    dq tree_find_parent
    db 'tree_find_parent', 0

    extern tree_find_parent_recur
    dq tree_find_parent_recur
    db 'tree_find_parent_recur', 0

    extern tree_insert
    dq tree_insert
    db 'tree_insert', 0

    extern tree_remove
    dq tree_remove
    db 'tree_remove', 0

    extern treenode_create
    dq treenode_create
    db 'treenode_create', 0

    extern vfs_close
    dq vfs_close
    db 'vfs_close', 0

    extern vfs_finddir
    dq vfs_finddir
    db 'vfs_finddir', 0

    extern vfs_fstat
    dq vfs_fstat
    db 'vfs_fstat', 0

    extern vfs_init
    dq vfs_init
    db 'vfs_init', 0

    extern vfs_lseek
    dq vfs_lseek
    db 'vfs_lseek', 0

    extern vfs_mount
    dq vfs_mount
    db 'vfs_mount', 0

    extern vfs_open
    dq vfs_open
    db 'vfs_open', 0

    extern vfs_read
    dq vfs_read
    db 'vfs_read', 0

    extern vfs_remove_dot_chars
    dq vfs_remove_dot_chars
    db 'vfs_remove_dot_chars', 0

    extern vfs_tree
    dq vfs_tree
    db 'vfs_tree', 0

    extern vfs_write
    dq vfs_write
    db 'vfs_write', 0

    extern vmm_init
    dq vmm_init
    db 'vmm_init', 0

    extern vmm_map_pages
    dq vmm_map_pages
    db 'vmm_map_pages', 0

    extern vmm_map_pages_huge
    dq vmm_map_pages_huge
    db 'vmm_map_pages_huge', 0

    extern vmm_unmap_pages
    dq vmm_unmap_pages
    db 'vmm_unmap_pages', 0

    extern vmm_unmap_pages_huge
    dq vmm_unmap_pages_huge
    db 'vmm_unmap_pages_huge', 0
global kernel_symbols_end
kernel_symbols_end: ;
