let genQRData = (() => {
    let canRun = false;
    Module.onRuntimeInitialized = () => canRun = true;
    const BUFFER_LENGTH = 32768;
    let QRData;
    let output_ptr;
    let gen_qr_code;
    let gen_qr = function (message, ECL) {
        if (canRun) {
            QRData = new Int8Array(BUFFER_LENGTH);
            gen_qr_code = Module.cwrap("gen_qr", null, ["number", "string", "number"]);
            output_ptr = Module._malloc(QRData.length);
            Module.HEAP8.set(QRData, output_ptr);
            has_init = true;
            gen_qr_code(ECL, message, output_ptr);
            const result = {
                size: undefined,
                QRData: undefined
            };
            const size = Module.getValue(output_ptr, "i8");
            result.size = size;
            result.QRData = [];
            for (let i = 0; i < size; i++) {
                let row = [];
                for (let j = 0; j < size; j++)
                    row.push(+Module.getValue(output_ptr + 1 + i * size + j, "i8") & 1);
                result.QRData.push(row);
            }
            Module._free(output_ptr);
            QRData = null;
            return result;
        } else
            return null;
    }
    return gen_qr;
})();
const _options = {
    message: "Hello World",
    ECL: 0,
    width: 200,
    fgColor: "black",
    bgColor: "white",
    borderWidth: 1,
    id: undefined,
    parent: undefined //can be a string representing an id or an object
};

const _ECL = {
    L: 0,
    M: 1,
    Q: 2,
    H: 3,
}

class QRCode {
    constructor(options = _options) {
        this.options = options;
        this.canvas = document.createElement("canvas");
        if (options.id)
            this.canvas.id = options.id;
        if (options.parent)
            this.appendFirst(options.parent);
        this.QRData;
    }

    drawToCanvas(width = this.options.width, canvas = this.canvas) {
        let ctx = canvas.getContext("2d");
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        ctx.beginPath();
        let size = this.QRData.size;
        canvas.width = width;
        canvas.height = width;
        let moduleWidth = Math.floor(width / size);
        this.options.borderWidth = Math.floor((width - moduleWidth * size) / 2);
        let offset = this.options.borderWidth;
        ctx.fillStyle = this.options.bgColor;
        ctx.fillRect(0, 0, width, width);
        ctx.fillStyle = this.options.fgColor;
        for (let i = 0; i < size; i++)
            for (let j = 0; j < size; j++) {
                if (this.QRData.QRData[i][j])
                    ctx.fillRect(offset + j * moduleWidth, offset + i * moduleWidth, moduleWidth, moduleWidth);
            }
        ctx.closePath();
    }
    getCanvas() {
        return this.canvas;
    }
    clear() {
        this.canvas.getContext("2d").clearRect(0, 0, this.canvas.width, this.canvas.height);
    }

    draw(message, ECL, fgColor = this.options.fgColor, bgColor = this.options.bgColor, width = this.options.width) {
        this.clear();
        this.options.message = message;
        this.options.ECL = ECL;
        this.options.fgColor = fgColor;
        this.options.bgColor = bgColor;
        this.options.width = width;
        this.QRData = genQRData(message, ECL);
        this.drawToCanvas(width > 200 ? 200 : width);
    }
    changeECL(ECL) {
        this.draw(this.options.message, ECL);
    }

    getCanvas() {
        return this.canvas;
    }
    changeFGColor(fgColor) {
        this.options.fgColor = fgColor;
        this.genQRAndDraw();
    }
    changeBGColor(bgColor) {
        this.options.bgColor = bgColor;
        this.genQRAndDraw();
    }
    setColor(bgColor, fgColor) {
        this.options.bgColor = bgColor;
        this.options.fgColor = fgColor;
        this.genQRAndDraw();
    }
    genQRAndDraw(canDraw = true) {
        let func = (resolve, reject) => {
            this.QRData = genQRData(this.options.message, this.options.ECL);
            if (!this.QRData)
                setTimeout(() => func(resolve), 10);
            else
                resolve();
        }
        if (!this.QRData) {
            new Promise(func).then(() => {
                if (canDraw)
                    this.drawToCanvas(this.options.width > 200 ? 200 : this.options.width);
            });
        }
        if (!!this.QRData && canDraw)
            this.drawToCanvas(this.options.width > 200 ? 200 : this.options.width);
    }

    appendToParent(node) {
        if (typeof node === "string")
            document.getElementById(node).appendChild(this.canvas);
        else if (typeof node === "object")
            node.appendChild(this.canvas);
    }

    appendFirst(node) {
        if (typeof node === "string")
            document.getElementById(node).prepend(this.canvas);
        else if (typeof node === "object")
            node.prepend(this.canvas);
    }

    generateSVG() {
        this.genQRAndDraw(false)
        let svg = document.createElementNS("http://www.w3.org/2000/svg", "svg");
        svg.setAttribute("width", this.options.width);
        svg.setAttribute("height", this.options.width);
        let bg = document.createElementNS("http://www.w3.org/2000/svg", "rect");
        bg.setAttribute("width", this.QRData.size);
        bg.setAttribute("height", this.QRData.size);
        bg.setAttribute("fill", this.options.bgColor);
        bg.setAttribute("stroke", this.options.bgColor);
        bg.setAttribute("stroke-width", this.options.borderWidth);
        bg.setAttribute("x", this.options.borderWidth);
        bg.setAttribute("y", this.options.borderWidth);
        svg.appendChild(bg);
        svg.setAttribute("viewBox", `0 0 ${this.QRData.size + this.options.borderWidth * 2} ${this.QRData.size + this.options.borderWidth * 2}`);
        let group = document.createElementNS("http://www.w3.org/2000/svg", "g");
        group.setAttribute("fill", this.options.fgColor);
        for (let i = 0; i < this.QRData.size; i++) {
            for (let j = 0; j < this.QRData.size; j++) {
                if (this.QRData.QRData[i][j]) {
                    let rect = document.createElementNS("http://www.w3.org/2000/svg", "rect");
                    rect.setAttribute("width", 1);
                    rect.setAttribute("height", 1);
                    rect.setAttribute("x", j + this.options.borderWidth);
                    rect.setAttribute("y", i + this.options.borderWidth);
                    group.appendChild(rect);
                }
            }
        }
        svg.appendChild(group);
        return svg;
    }
    getQRCodeURI() {
        let canvas = document.createElement("canvas");
        this.drawToCanvas(this.options.width, canvas);
        return canvas.toDataURL("image/png:base64");
    }
}

QRCode.options = _options;
QRCode.ECL = _ECL;