
{
    message = $0

    message_length = length(message)

    length_bytes = sprintf("%c%c%c%c", message_length % 256, int(message_length / 256) % 256, int(message_length / 65536) % 256, int(message_length / 16777216) % 256)

    full_message = length_bytes message

    print(full_message)

    fflush(stdout)
}
    