.extern irq_handler

.macro TRAP intno
trap\intno:
    cli
    pushl $0x00
    pushl $\intno
    jmp irq_common
.endm

.macro TRAP_ERROR intno 
trap\intno:
    cli
    #already pushed
    pushl $\intno
    jmp irq_common
.endm

/*
  Stack content right before call interrupt_handler

  +=========+
  |  ss     |   <- pushed only if a privelege level change occurs
  +=========+
  |  esp    |
  +=========+
  |  eflgs  |
  +=========+
  |  cs     |
  +=========+
  |  eip    |
  +=========+
  |  error  |
  +=========+
  |  int nr |
  +=========+
  |  eax    |
  +=========+
  |  ecx    |
  +=========+
  |  edx    |
  +=========+
  |  ebx    |
  +=========+
  |  esp    |
  +=========+
  |  ebp    |
  +=========+
  |  esi    |
  +=========+
  |  edi    |
  +=========+
  |  ds     |
  +=========+   <- esp

*/

.text

irq_common:
    pusha

    movw %ds, %ax
    pushl %eax

    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    pushl %esp
    call irq_handler
    addl $4, %esp

.globl irq_end
irq_end:
    popl %eax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    popa
    addl $0x8, %esp

    sti
    iret

TRAP 0
TRAP 1
TRAP 2
TRAP 3
TRAP 4
TRAP 5
TRAP 6
TRAP 7
TRAP_ERROR 8
TRAP 9
TRAP_ERROR 10
TRAP_ERROR 11
TRAP_ERROR 12
TRAP_ERROR 13
TRAP_ERROR 14
TRAP 15
TRAP 16
TRAP 17
TRAP 18
TRAP 19
TRAP 20
TRAP 21
TRAP 22
TRAP 23
TRAP 24
TRAP 25
TRAP 26
TRAP 27
TRAP 28
TRAP 29
TRAP 30
TRAP 31
TRAP 32
TRAP 33
TRAP 34
TRAP 35
TRAP 36
TRAP 37
TRAP 38
TRAP 39
TRAP 40
TRAP 41
TRAP 42
TRAP 43
TRAP 44
TRAP 45
TRAP 46
TRAP 47
TRAP 48
TRAP 49
TRAP 50
TRAP 51
TRAP 52
TRAP 53
TRAP 54
TRAP 55
TRAP 56
TRAP 57
TRAP 58
TRAP 59
TRAP 60
TRAP 61
TRAP 62
TRAP 63
TRAP 64
TRAP 65
TRAP 66
TRAP 67
TRAP 68
TRAP 69
TRAP 70
TRAP 71
TRAP 72
TRAP 73
TRAP 74
TRAP 75
TRAP 76
TRAP 77
TRAP 78
TRAP 79
TRAP 80
TRAP 81
TRAP 82
TRAP 83
TRAP 84
TRAP 85
TRAP 86
TRAP 87
TRAP 88
TRAP 89
TRAP 90
TRAP 91
TRAP 92
TRAP 93
TRAP 94
TRAP 95
TRAP 96
TRAP 97
TRAP 98
TRAP 99
TRAP 100
TRAP 101
TRAP 102
TRAP 103
TRAP 104
TRAP 105
TRAP 106
TRAP 107
TRAP 108
TRAP 109
TRAP 110
TRAP 111
TRAP 112
TRAP 113
TRAP 114
TRAP 115
TRAP 116
TRAP 117
TRAP 118
TRAP 119
TRAP 120
TRAP 121
TRAP 122
TRAP 123
TRAP 124
TRAP 125
TRAP 126
TRAP 127
TRAP 128
TRAP 129
TRAP 130
TRAP 131
TRAP 132
TRAP 133
TRAP 134
TRAP 135
TRAP 136
TRAP 137
TRAP 138
TRAP 139
TRAP 140
TRAP 141
TRAP 142
TRAP 143
TRAP 144
TRAP 145
TRAP 146
TRAP 147
TRAP 148
TRAP 149
TRAP 150
TRAP 151
TRAP 152
TRAP 153
TRAP 154
TRAP 155
TRAP 156
TRAP 157
TRAP 158
TRAP 159
TRAP 160
TRAP 161
TRAP 162
TRAP 163
TRAP 164
TRAP 165
TRAP 166
TRAP 167
TRAP 168
TRAP 169
TRAP 170
TRAP 171
TRAP 172
TRAP 173
TRAP 174
TRAP 175
TRAP 176
TRAP 177
TRAP 178
TRAP 179
TRAP 180
TRAP 181
TRAP 182
TRAP 183
TRAP 184
TRAP 185
TRAP 186
TRAP 187
TRAP 188
TRAP 189
TRAP 190
TRAP 191
TRAP 192
TRAP 193
TRAP 194
TRAP 195
TRAP 196
TRAP 197
TRAP 198
TRAP 199
TRAP 200
TRAP 201
TRAP 202
TRAP 203
TRAP 204
TRAP 205
TRAP 206
TRAP 207
TRAP 208
TRAP 209
TRAP 210
TRAP 211
TRAP 212
TRAP 213
TRAP 214
TRAP 215
TRAP 216
TRAP 217
TRAP 218
TRAP 219
TRAP 220
TRAP 221
TRAP 222
TRAP 223
TRAP 224
TRAP 225
TRAP 226
TRAP 227
TRAP 228
TRAP 229
TRAP 230
TRAP 231
TRAP 232
TRAP 233
TRAP 234
TRAP 235
TRAP 236
TRAP 237
TRAP 238
TRAP 239
TRAP 240
TRAP 241
TRAP 242
TRAP 243
TRAP 244
TRAP 245
TRAP 246
TRAP 247
TRAP 248
TRAP 249
TRAP 250
TRAP 251
TRAP 252
TRAP 253
TRAP 254
TRAP 255

.data
.globl trap_vector
trap_vector:
    .long trap0
    .long trap1
    .long trap2
    .long trap3
    .long trap4
    .long trap5
    .long trap6
    .long trap7
    .long trap8
    .long trap9
    .long trap10
    .long trap11
    .long trap12
    .long trap13
    .long trap14
    .long trap15
    .long trap16
    .long trap17
    .long trap18
    .long trap19
    .long trap20
    .long trap21
    .long trap22
    .long trap23
    .long trap24
    .long trap25
    .long trap26
    .long trap27
    .long trap28
    .long trap29
    .long trap30
    .long trap31
    .long trap32
    .long trap33
    .long trap34
    .long trap35
    .long trap36
    .long trap37
    .long trap38
    .long trap39
    .long trap40
    .long trap41
    .long trap42
    .long trap43
    .long trap44
    .long trap45
    .long trap46
    .long trap47
    .long trap48
    .long trap49
    .long trap50
    .long trap51
    .long trap52
    .long trap53
    .long trap54
    .long trap55
    .long trap56
    .long trap57
    .long trap58
    .long trap59
    .long trap60
    .long trap61
    .long trap62
    .long trap63
    .long trap64
    .long trap65
    .long trap66
    .long trap67
    .long trap68
    .long trap69
    .long trap70
    .long trap71
    .long trap72
    .long trap73
    .long trap74
    .long trap75
    .long trap76
    .long trap77
    .long trap78
    .long trap79
    .long trap80
    .long trap81
    .long trap82
    .long trap83
    .long trap84
    .long trap85
    .long trap86
    .long trap87
    .long trap88
    .long trap89
    .long trap90
    .long trap91
    .long trap92
    .long trap93
    .long trap94
    .long trap95
    .long trap96
    .long trap97
    .long trap98
    .long trap99
    .long trap100
    .long trap101
    .long trap102
    .long trap103
    .long trap104
    .long trap105
    .long trap106
    .long trap107
    .long trap108
    .long trap109
    .long trap110
    .long trap111
    .long trap112
    .long trap113
    .long trap114
    .long trap115
    .long trap116
    .long trap117
    .long trap118
    .long trap119
    .long trap120
    .long trap121
    .long trap122
    .long trap123
    .long trap124
    .long trap125
    .long trap126
    .long trap127
    .long trap128
    .long trap129
    .long trap130
    .long trap131
    .long trap132
    .long trap133
    .long trap134
    .long trap135
    .long trap136
    .long trap137
    .long trap138
    .long trap139
    .long trap140
    .long trap141
    .long trap142
    .long trap143
    .long trap144
    .long trap145
    .long trap146
    .long trap147
    .long trap148
    .long trap149
    .long trap150
    .long trap151
    .long trap152
    .long trap153
    .long trap154
    .long trap155
    .long trap156
    .long trap157
    .long trap158
    .long trap159
    .long trap160
    .long trap161
    .long trap162
    .long trap163
    .long trap164
    .long trap165
    .long trap166
    .long trap167
    .long trap168
    .long trap169
    .long trap170
    .long trap171
    .long trap172
    .long trap173
    .long trap174
    .long trap175
    .long trap176
    .long trap177
    .long trap178
    .long trap179
    .long trap180
    .long trap181
    .long trap182
    .long trap183
    .long trap184
    .long trap185
    .long trap186
    .long trap187
    .long trap188
    .long trap189
    .long trap190
    .long trap191
    .long trap192
    .long trap193
    .long trap194
    .long trap195
    .long trap196
    .long trap197
    .long trap198
    .long trap199
    .long trap200
    .long trap201
    .long trap202
    .long trap203
    .long trap204
    .long trap205
    .long trap206
    .long trap207
    .long trap208
    .long trap209
    .long trap210
    .long trap211
    .long trap212
    .long trap213
    .long trap214
    .long trap215
    .long trap216
    .long trap217
    .long trap218
    .long trap219
    .long trap220
    .long trap221
    .long trap222
    .long trap223
    .long trap224
    .long trap225
    .long trap226
    .long trap227
    .long trap228
    .long trap229
    .long trap230
    .long trap231
    .long trap232
    .long trap233
    .long trap234
    .long trap235
    .long trap236
    .long trap237
    .long trap238
    .long trap239
    .long trap240
    .long trap241
    .long trap242
    .long trap243
    .long trap244
    .long trap245
    .long trap246
    .long trap247
    .long trap248
    .long trap249
    .long trap250
    .long trap251
    .long trap252
    .long trap253
    .long trap254
    .long trap255
