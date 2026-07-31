package com.example.smarthome

import android.graphics.Color
import android.os.Bundle
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.cardview.widget.CardView
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.*
import java.io.BufferedReader
import java.io.InputStreamReader
import java.net.Socket

class MainActivity : AppCompatActivity() {

    // UI 组件声明
    private lateinit var etIpAddress: EditText
    private lateinit var etPort: EditText
    private lateinit var btnConnect: Button
    private lateinit var tvStatus: TextView

    private lateinit var cardDoor: CardView
    private lateinit var cardIr: CardView

    private lateinit var tvTemp: TextView
    private lateinit var tvHum: TextView
    private lateinit var tvSmoke: TextView
    private lateinit var tvGas: TextView

    // 连接状态控制
    private var isConnected = false
    private var tcpSocket: Socket? = null

    // 预设颜色
    private val colorNormal = Color.parseColor("#4CAF50") // 绿色
    private val colorAlert = Color.parseColor("#F44336")  // 红色
    private val colorOffline = Color.parseColor("#9E9E9E")// 灰色

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // 绑定 UI 组件
        etIpAddress = findViewById(R.id.etIpAddress)
        etPort = findViewById(R.id.etPort)
        btnConnect = findViewById(R.id.btnConnect)
        tvStatus = findViewById(R.id.tvStatus)

        cardDoor = findViewById(R.id.cardDoor)
        cardIr = findViewById(R.id.cardIr)

        tvTemp = findViewById(R.id.tvTemp)
        tvHum = findViewById(R.id.tvHum)
        tvSmoke = findViewById(R.id.tvSmoke)
        tvGas = findViewById(R.id.tvGas)

        // 按钮点击事件
        btnConnect.setOnClickListener {
            if (isConnected) {
                disconnectDevice()
            } else {
                val ip = etIpAddress.text.toString()
                val port = etPort.text.toString().toIntOrNull() ?: 8080
                connectDevice(ip, port)
            }
        }
    }

    private fun connectDevice(ip: String, port: Int) {
        btnConnect.text = "连 接 中 ..."
        btnConnect.isEnabled = false

        lifecycleScope.launch(Dispatchers.IO) {
            try {
                tcpSocket = Socket(ip, port)
                isConnected = true

                withContext(Dispatchers.Main) {
                    btnConnect.text = "断 开 连 接"
                    btnConnect.isEnabled = true
                    tvStatus.text = "已连接: $ip:$port"
                    tvStatus.setTextColor(Color.GREEN)
                }

                val reader = BufferedReader(InputStreamReader(tcpSocket!!.getInputStream()))
                while (isActive && isConnected) {
                    val line = reader.readLine() ?: break
                    withContext(Dispatchers.Main) {
                        parseAndDisplay(line)
                    }
                }
            } catch (e: Exception) {
                // 连接失败或断开
            } finally {
                disconnectDevice()
            }
        }
    }

    private fun disconnectDevice() {
        isConnected = false
        try {
            tcpSocket?.close()
        } catch (e: Exception) { }

        // 回到主线程恢复 UI 为初始状态
        lifecycleScope.launch(Dispatchers.Main) {
            btnConnect.text = "连 接 设 备"
            btnConnect.isEnabled = true
            tvStatus.text = "当前状态: 未连接"
            tvStatus.setTextColor(Color.GRAY)

            // 恢复卡片为灰色
            cardDoor.setCardBackgroundColor(colorOffline)
            cardIr.setCardBackgroundColor(colorOffline)

            tvTemp.text = "-- ℃"
            tvHum.text = "-- %"
            tvSmoke.text = "等待数据"
            tvGas.text = "等待数据"
        }
    }

    private fun parseAndDisplay(data: String) {
        try {
            // 假设收到的数据格式为: "门磁,红外,温度,湿度,烟雾,燃气"
            // 例如: "0,1,25.6,40.0,0,0"
            val parts = data.trim().split(",")
            if (parts.size >= 6) {
                val doorVal = parts[0]
                val irVal = parts[1]
                val tempVal = parts[2]
                val humVal = parts[3]
                val smokeVal = parts[4]
                val gasVal = parts[5]

                // 1. 更新门磁、红外卡片颜色 (0绿, 1红)
                cardDoor.setCardBackgroundColor(if (doorVal == "1") colorAlert else colorNormal)
                cardIr.setCardBackgroundColor(if (irVal == "1") colorAlert else colorNormal)

                // 2. 更新温湿度
                tvTemp.text = "$tempVal ℃"
                tvHum.text = "$humVal %"

                // 3. 更新烟雾、燃气文字状态
                tvSmoke.text = if (smokeVal == "1") "⚠️ 报警" else "安全"
                tvSmoke.setTextColor(if (smokeVal == "1") colorAlert else colorNormal)

                tvGas.text = if (gasVal == "1") "⚠️ 泄漏" else "安全"
                tvGas.setTextColor(if (gasVal == "1") colorAlert else colorNormal)
            }
        } catch (e: Exception) {
            // 解析错误时忽略，防止崩溃
        }
    }
}