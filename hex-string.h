String hexString(
  uint8_t *a,
  int len
) {
  String r = "";
  for (int i = 0; i < len; i++) {
    if (a[i] < 0x10)
      r += '0';
    r += String(a[i], HEX);
  }
  return r;
}
