let func = (function QRCode() {
    let canRun = false;
    Module.onRuntimeInitialized = () => canRun = true;
    const BUFFER_LENGTH = 32768;
    let QRData;
    let output_ptr;
    let gen_qr_code;
    let has_init = true;
    let gen_qr = function (message, ECL) {
        if (canRun) {
            if (has_init) {
                QRData = new Int8Array(BUFFER_LENGTH);
                gen_qr_code = Module.cwrap("gen_qr", null, ["number", "string", "number"]);
                output_ptr = Module._malloc(QRData.length);
                Module.HEAP8.set(QRData, output_ptr);
                has_init = false;
            }
            gen_qr_code(ECL, message, output_ptr);
            const result = {size: undefined, QRData: undefined};
            const size = Module.getValue(output_ptr, "i8");
            result.size = size;
            result.QRData = [];
            for (let i = 0; i < size; i++) {
                let row = [];
                for (let j = 0; j < size; j++) 
                    row.push(+Module.getValue(output_ptr + 1 + i * size + j, "i8") & 1);
                result.QRData.push(row);
            }
            // Module._free(output_ptr);
            QRData = null;
            return result;
        }
        return null;
    }
    return gen_qr;
})();

setInterval(() => {
    let a = func("Hello World", 0);
}, 1000);