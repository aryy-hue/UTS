// Fungsi untuk mengambil data dari API
async function fetchData() {
    try {
        const response = await fetch('http://localhost:3000/data'); // Pastikan URL API benar
        const data = await response.json();

        // Tampilkan data suhu maksimum, minimum, dan rata-rata
        document.getElementById('suhumax').textContent = data.suhumax;
        document.getElementById('suhumin').textContent = data.suhumin;
        document.getElementById('suhurata').textContent = data.suhurata.toFixed(2);

        // Tampilkan data suhu dan kelembapan terbaru di tabel
        const tableBody = document.getElementById('data-table');
        tableBody.innerHTML = '';

        data.nilai_suhu_max_humid_max.forEach(item => {
            const row = document.createElement('tr');
            row.innerHTML = `
                <td>${new Date(item.timestamp).toLocaleString()}</td>
                <td>${item.suhun}</td>
                <td>${item.humid}</td>
                <td>${item.kecerahan}</td>
            `;
            tableBody.appendChild(row);
        });
    } catch (error) {
        console.error('Error fetching data:', error);
    }
}

// Panggil fetchData saat halaman dimuat
document.addEventListener('DOMContentLoaded', fetchData);
