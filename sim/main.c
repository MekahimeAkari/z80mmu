#include <stdio.h>
#include <string.h>
#include <Z/constants/pointer.h>
#include <Z80.h>

#define CYCLES_PER_FRAME 69888
#define ROM_SIZE         0x4000 /* 16 KiB */
#define MEMORY_SIZE      0x8000 /* 32 KiB */

typedef struct _Device {
        void* context;
        zuint8 (* read)(void *context);
        void (* write)(void *context, zuint8 value);
        zuint16 assigned_port;
} Device;

typedef struct _Machine {
        zusize  cycles;
        zuint8  memory[65536];
        Z80     cpu;
        Device* devices;
        zusize  device_count;
} Machine;


Device *machine_find_device(Machine *self, zuint16 port)
{
    zusize index = 0;

    for (; index < self->device_count; index++)
            if (self->devices[index].assigned_port == port)
                    return &self->devices[index];

    return Z_NULL;
}


static zuint8 machine_cpu_read(Machine *self, zuint16 address)
{
    printf("helo at 0x%x\n", address);
    return address < MEMORY_SIZE ? self->memory[address] : 0xFF;
}


static void machine_cpu_write(Machine *self, zuint16 address, zuint8 value)
{
    if (address >= ROM_SIZE && address < MEMORY_SIZE)
        self->memory[address] = value;
}


static zuint8 machine_cpu_in(Machine *self, zuint16 port)
{
    Device *device = machine_find_device(self, port);

    return device != Z_NULL ? device->read(device->context) : 0xFF;
}


static void machine_cpu_out(Machine *self, zuint16 port, zuint8 value)
{
    Device *device = machine_find_device(self, port);

    if (device != Z_NULL) device->write(device->context, value);
}

static zbool halted = 0;

static void machine_halt(Machine *self, zuint8 signal)
{
    halted = 1;
}

void machine_init(Machine *self)
{
    self->cpu.context      = self;
    self->cpu.fetch_opcode =
    self->cpu.fetch        =
    self->cpu.nop          =
    self->cpu.read         = (Z80Read )machine_cpu_read;
    self->cpu.write        = (Z80Write)machine_cpu_write;
    self->cpu.in           = (Z80Read )machine_cpu_in;
    self->cpu.out          = (Z80Write)machine_cpu_out;
    self->cpu.halt         = (Z80Halt)machine_halt;
    self->cpu.nmia         = Z_NULL;
    self->cpu.inta         = Z_NULL;
    self->cpu.int_fetch    = Z_NULL;
    self->cpu.ld_i_a       = Z_NULL;
    self->cpu.ld_r_a       = Z_NULL;
    self->cpu.reti         = Z_NULL;
    self->cpu.retn         = Z_NULL;
    self->cpu.hook         = Z_NULL;
    self->cpu.illegal      = Z_NULL;
    self->cpu.options      = Z80_MODEL_ZILOG_NMOS;
}


void machine_power(Machine *self, zbool state)
{
    if (state)
    {
        machine_init(self);
        self->cycles = 0;
        memset(self->memory, 0, 65536);
    }

    z80_power(&self->cpu, state);
}


void machine_reset(Machine *self)
{
    z80_instant_reset(&self->cpu);
}


void machine_run_frame(Machine *self)
{
    zusize cycles = 0;
    do
    {
        cycles += z80_execute(&self->cpu, 1);
        printf("Cycle: %lu\n", cycles);
        if (halted == 1)
            break;
    }
    while (cycles < CYCLES_PER_FRAME);
}

int load_binary(Machine *self, const char *bin_path)
{
    FILE *bin_file = fopen(bin_path, "r");
    if (bin_file == NULL)
    {
        perror("Error opening file");
        return 1;
    }
    char c = fgetc(bin_file);
    int addr = 0;
    while (c != EOF && addr < MEMORY_SIZE)
    {
        printf("0x%x: 0x%x\n", addr, c);
        self->memory[addr] = c;
        addr++;
        c = fgetc(bin_file);
    }
    fclose(bin_file);
    return 0;
}

int main()
{
    printf("henlo\n");
    Machine z80_cpu;
    machine_power(&z80_cpu, 1);
    if(load_binary(&z80_cpu, "test.bin") != 0)
        return 1;
    machine_run_frame(&z80_cpu);
}
