#include "CPU.h"
#include "Pipeline.h"
#include <stdio.h>
#include <stdlib.h>

struct BusController;

/* Stub functions. */
uint32_t BusReadWord(struct BusController *unused(bus),
  uint32_t unused(address)) { return 0; }
void BusWriteWord(const struct BusController *unused(bus),
  uint32_t unused(address), uint32_t unused(size)) {}

void DMAFromDRAM(struct BusController *unused(bus), void *unused(dest),
  uint32_t unused(src), uint32_t unused(size)) {}
void DMAToDRAM(struct BusController *unused(bus), uint32_t unused(dest),
  const void *unused(src), size_t unused(size)) {}

void BusClearRCPInterrupt(struct BusController *unused(bus),
  unsigned unused(i)) {}
void BusRaiseRCPInterrupt(struct BusController *unused(bus),
  unsigned unused(i)) {}

int DPRegRead(void *unused(rdp), uint32_t unused(address),
  void *unused(data)) { return 0; }
int DPRegWrite(void *unused(rdp), uint32_t unused(address),
  void *unused(data)) { return 0; }

/* Entry point. */
int main(int argc, const char *argv[]) {
	FILE *rspUCodeFile;
	struct RSP *rsp;
	size_t total, size;
	long i, cycles;

	if (argc != 3) {
		printf("Usage: %s <uCode> <Cycles>\n", argv[0]);
		return 0;
	}

	/* Open the uCode file, create an RSP instance. */
	if ((rspUCodeFile = fopen(argv[1], "rb")) == NULL) {
		printf("Failed to open RSP uCode.\n");

		return 1;
	}

	if ((rsp = CreateRSP()) == NULL) {
		printf("Failed to initialize the RSP.\n");

		fclose(rspUCodeFile);
		return 2;
	}

	/* Read the uCode into the RSP. */
	for (size = 0, total = 0; total < 4096; total += size) {
		size = fread(rsp->imem + total, 1, 4096 - total, rspUCodeFile);

		if (ferror(rspUCodeFile)) {
			printf("Unable to read the uCode file.\n");

			fclose(rspUCodeFile);
			DestroyRSP(rsp);
			return 3;
		}
	}

	/* Read the uCode into the RSP. */
	for (size = 0, total = 0; total < 4096; total += size) {
		size = fread(rsp->dmem + total, 1, 4096 - total, rspUCodeFile);

		if (ferror(rspUCodeFile)) {
			printf("Unable to read the uCode file.\n");

			fclose(rspUCodeFile);
			DestroyRSP(rsp);
			return 3;
		}
	}

	fclose(rspUCodeFile);
	cycles = strtol(argv[2], NULL, 10);

	printf("Running RSP for %ld cycles.\n", cycles);
  rsp->cp0.regs[SP_STATUS_REG] = 0; /* Unhalt. */

	for (i = 0; i < cycles; i++)
		CycleRSP(rsp);

	RSPDumpRegisters(rsp);
	return 0;
}

