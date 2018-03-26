/* RFC 1071
        Depending upon the machine, it may be more efficient to defer
        adding end-around carries until the main summation loop is
        finished.

        One approach is to sum 16-bit words in a 32-bit accumulator, so
        the overflows build up in the high-order 16 bits.  This approach
        typically avoids a carry-sensing instruction but requires twice
        as many additions as would adding 32-bit segments; which is
        faster depends upon the detailed hardware architecture.
*/
unsigned short internet_checksum(unsigned short *addr, int count) {

    register int sum = 0;

    while (count > 1) {
        sum += *addr++;
        count -= 2;
    }

    if (count > 0) {
        sum +=   * (unsigned char *) addr;
    }

    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }

    return (~sum);
}
