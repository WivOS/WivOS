#pragma once

#include <modules/modules.h>
#include <util/util.h>
#include <cpu/pci.h>

typedef struct {
    uint16_t mode;
    uint8_t rlen;
    uint8_t tlen;
    uint8_t mac[6];
    uint16_t reserved;
    uint8_t ladr[8];
    uint32_t physRx;
    uint32_t physTx;
} __attribute__((packed)) config_bytes_t;

typedef struct
{
    uint8_t n[6];
} __attribute__((packed)) ethernet_addr_t;

typedef struct {
    ethernet_addr_t destMac;
    ethernet_addr_t srcMac;
    uint16_t ethType;
} __attribute__((packed)) ethernet_header_t;

typedef struct {
    uint16_t htype;
    uint16_t ptype;
    uint8_t hlen;
    uint8_t plen;
    uint16_t opcode;
    uint8_t srcHw[6];
    uint8_t srcPr[4];
    uint8_t dstHw[6];
    uint8_t dstPr[4];
} __attribute__((packed)) arp_header_t;

typedef struct {
    uint8_t versionIHL;
    uint8_t tos;
    uint16_t length;
    uint16_t indent;
    uint16_t flagsFragmentOffset;
    uint8_t timeToLive;
    uint8_t protocol;
    uint16_t checksum;
    uint8_t srcIp[4];
    uint8_t dstIp[4];
} __attribute__((packed)) ipv4_header_t;

typedef struct {
    uint8_t icmpType;
    uint8_t icmpCode;
    uint16_t checksum;
    uint16_t echoId;
    uint16_t echoSeq;
} __attribute__((packed)) icmpr_header_t;
