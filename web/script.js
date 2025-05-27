class ConfigManager {
    constructor() {
        this.apiBase = '/api';
        this.init();
    }

    async init() {
        this.setupEventListeners();
        await this.loadConfig();
    }

    setupEventListeners() {
        // Load config button
        document.getElementById('load-config').addEventListener('click', () => {
            this.loadConfig();
        });

        // Save config button
        document.getElementById('save-config').addEventListener('click', () => {
            this.saveConfig();
        });

        // Real-time slider updates
        const sliders = document.querySelectorAll('input[type="range"]');
        sliders.forEach(slider => {
            slider.addEventListener('input', (e) => {
                this.updateSliderDisplay(e.target);
            });
        });

        // Initialize slider displays
        sliders.forEach(slider => {
            this.updateSliderDisplay(slider);
        });
    }

    updateSliderDisplay(slider) {
        const valueSpan = document.getElementById(slider.id + '-value');
        if (valueSpan) {
            valueSpan.textContent = slider.value;
        }
    }

    async loadConfig() {
        try {
            this.setStatus('Loading configuration...', 'info');

            const response = await fetch(`${this.apiBase}/config`);
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }

            const config = await response.json();
            this.populateForm(config);
            this.setStatus('Configuration loaded successfully!', 'success');

        } catch (error) {
            console.error('Error loading config:', error);
            this.setStatus(`Error loading configuration: ${error.message}`, 'error');
        }
    }

    async saveConfig() {
        try {
            this.setStatus('Saving configuration...', 'info');

            const config = this.getFormData();

            const response = await fetch(`${this.apiBase}/config`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(config)
            });

            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }

            const result = await response.json();
            if (result.status === 'success') {
                this.setStatus('Configuration saved successfully!', 'success');
            } else {
                throw new Error(result.message || 'Unknown error');
            }

        } catch (error) {
            console.error('Error saving config:', error);
            this.setStatus(`Error saving configuration: ${error.message}`, 'error');
        }
    }

    populateForm(config) {
        // NDI settings
        document.getElementById('ndi-source').value = config.ndi_source_name || '';

        // Color filter settings
        document.getElementById('hue-min').value = config.hue_min || 0;
        document.getElementById('hue-max').value = config.hue_max || 180;
        document.getElementById('sat-min').value = config.saturation_min || 50;
        document.getElementById('sat-max').value = config.saturation_max || 255;
        document.getElementById('val-min').value = config.value_min || 50;
        document.getElementById('val-max').value = config.value_max || 255;

        // Mouse settings
        document.getElementById('sensitivity').value = config.mouse_sensitivity || 1.0;

        // USB settings
        document.getElementById('vendor-id').value = this.intToHex(config.vendor_id);
        document.getElementById('product-id').value = this.intToHex(config.product_id);

        // Update slider displays
        const sliders = document.querySelectorAll('input[type="range"]');
        sliders.forEach(slider => {
            this.updateSliderDisplay(slider);
        });
    }

    getFormData() {
        return {
            ndi_source_name: document.getElementById('ndi-source').value,
            hue_min: parseInt(document.getElementById('hue-min').value),
            hue_max: parseInt(document.getElementById('hue-max').value),
            saturation_min: parseInt(document.getElementById('sat-min').value),
            saturation_max: parseInt(document.getElementById('sat-max').value),
            value_min: parseInt(document.getElementById('val-min').value),
            value_max: parseInt(document.getElementById('val-max').value),
            mouse_sensitivity: parseFloat(document.getElementById('sensitivity').value),
            vendor_id: this.hexToInt(document.getElementById('vendor-id').value),
            product_id: this.hexToInt(document.getElementById('product-id').value)
        };
    }

    intToHex(value) {
        if (!value) return '0x0000';
        return '0x' + value.toString(16).toUpperCase().padStart(4, '0');
    }

    hexToInt(hexString) {
        if (!hexString) return 0;
        // Remove 0x prefix if present
        hexString = hexString.replace(/^0x/i, '');
        return parseInt(hexString, 16) || 0;
    }

    setStatus(message, type) {
        const statusDiv = document.getElementById('status-message');
        statusDiv.textContent = message;
        statusDiv.className = `status-${type}`;

        // Auto-clear success messages after 3 seconds
        if (type === 'success') {
            setTimeout(() => {
                statusDiv.textContent = '';
                statusDiv.className = '';
            }, 3000);
        }
    }
}

// Initialize when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    new ConfigManager();
});