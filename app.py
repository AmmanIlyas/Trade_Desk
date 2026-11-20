
import tkinter as tk
from tkinter import ttk, messagebox
import sqlite3
import random
import os
import csv
from datetime import datetime

# ==========================================
# DATA STORAGE ON YOUR WINDOWS DESKTOP
# ==========================================

DESKTOP = os.path.join(os.path.expanduser("~"), "Desktop")
DATA_FOLDER = os.path.join(DESKTOP, "TradingSystemData")
os.makedirs(DATA_FOLDER, exist_ok=True)

DB_NAME = os.path.join(DATA_FOLDER, "trading_system.db")


# ==========================================
# DATABASE
# ==========================================

def connect_db():
    return sqlite3.connect(DB_NAME)


def initialize_database():
    conn = connect_db()
    cur = conn.cursor()

    cur.execute("""
        CREATE TABLE IF NOT EXISTS stocks (
            symbol TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            price REAL NOT NULL,
            previous_price REAL NOT NULL
        )
    """)

    cur.execute("""
        CREATE TABLE IF NOT EXISTS portfolio (
            symbol TEXT PRIMARY KEY,
            quantity INTEGER NOT NULL DEFAULT 0
        )
    """)

    cur.execute("""
        CREATE TABLE IF NOT EXISTS orders (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            time TEXT,
            symbol TEXT,
            side TEXT,
            quantity INTEGER,
            price REAL,
            total REAL,
            status TEXT
        )
    """)

    cur.execute("""
        CREATE TABLE IF NOT EXISTS trades (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            time TEXT,
            symbol TEXT,
            side TEXT,
            quantity INTEGER,
            price REAL,
            total REAL
        )
    """)

    cur.execute("""
        CREATE TABLE IF NOT EXISTS settings (
            key TEXT PRIMARY KEY,
            value REAL
        )
    """)

    cur.execute("""
        INSERT OR IGNORE INTO settings (key, value)
        VALUES ('cash', 100000)
    """)

    stocks = [
        ("AAPL", "Apple Inc.", 225.50),
        ("MSFT", "Microsoft", 425.20),
        ("GOOGL", "Alphabet", 178.40),
        ("AMZN", "Amazon", 215.80),
        ("TSLA", "Tesla", 350.10),
        ("NVDA", "NVIDIA", 145.70),
        ("META", "Meta Platforms", 610.30),
        ("NFLX", "Netflix", 920.60),
        ("IBM", "IBM", 260.20),
        ("ORCL", "Oracle", 185.40)
    ]

    for symbol, name, price in stocks:
        cur.execute("""
            INSERT OR IGNORE INTO stocks
            (symbol, name, price, previous_price)
            VALUES (?, ?, ?, ?)
        """, (symbol, name, price, price))

    conn.commit()
    conn.close()


# ==========================================
# EXPORT RECORDS TO CSV AND TXT FILES
# ==========================================

def export_records():
    conn = connect_db()
    cur = conn.cursor()

    exports = [
        (
            "trades",
            "trade_history.csv",
            ["Trade ID", "Time", "Symbol", "Side",
             "Quantity", "Price", "Total"],
            """SELECT id, time, symbol, side, quantity,
                      price, total FROM trades ORDER BY id DESC"""
        ),
        (
            "orders",
            "orders.csv",
            ["Order ID", "Time", "Symbol", "Side",
             "Quantity", "Price", "Total", "Status"],
            """SELECT id, time, symbol, side, quantity,
                      price, total, status
               FROM orders ORDER BY id DESC"""
        ),
        (
            "portfolio",
            "portfolio.csv",
            ["Symbol", "Company", "Quantity",
             "Current Price", "Market Value"],
            """SELECT p.symbol, s.name, p.quantity, s.price,
                      p.quantity * s.price
               FROM portfolio p
               JOIN stocks s ON p.symbol = s.symbol"""
        ),
        (
            "stocks",
            "stocks.csv",
            ["Symbol", "Company", "Current Price", "Previous Price"],
            """SELECT symbol, name, price, previous_price FROM stocks"""
        )
    ]

    for _, filename, headings, query in exports:
        cur.execute(query)
        rows = cur.fetchall()

        filepath = os.path.join(DATA_FOLDER, filename)

        with open(filepath, "w", newline="", encoding="utf-8-sig") as file:
            writer = csv.writer(file)
            writer.writerow(headings)
            writer.writerows(rows)

    cur.execute("SELECT value FROM settings WHERE key = 'cash'")
    row = cur.fetchone()
    cash = float(row[0]) if row else 0

    with open(
        os.path.join(DATA_FOLDER, "account_balance.txt"),
        "w",
        encoding="utf-8"
    ) as file:
        file.write("STOCK TRADING SYSTEM ACCOUNT\n")
        file.write("============================\n")
        file.write(f"Available Cash: ${cash:,.2f}\n")
        file.write("Last Updated: ")
        file.write(datetime.now().strftime("%Y-%m-%d %H:%M:%S"))

    conn.close()


# ==========================================
# TRADING APPLICATION
# ==========================================

class TradingApp:

    def __init__(self, root):
        self.root = root
        self.root.title("Stock Trading System")
        self.root.geometry("1200x760")
        self.root.minsize(950, 650)

        self.selected_symbol = "AAPL"
        self.refresh_job = None

        self.setup_style()
        self.create_header()
        self.create_interface()

        self.refresh_all()
        self.update_market_prices()

        self.root.protocol("WM_DELETE_WINDOW", self.close_app)

    # ======================================
    # USER INTERFACE
    # ======================================

    def setup_style(self):
        style = ttk.Style()

        try:
            style.theme_use("clam")
        except tk.TclError:
            pass

        style.configure("Treeview", rowheight=25)
        style.configure(
            "Treeview.Heading",
            font=("Arial", 10, "bold")
        )

    def create_header(self):
        header = tk.Frame(self.root, bg="#172554", height=75)
        header.pack(fill="x")
        header.pack_propagate(False)

        tk.Label(
            header,
            text="STOCK TRADING SYSTEM",
            bg="#172554",
            fg="white",
            font=("Arial", 20, "bold")
        ).pack(side="left", padx=20, pady=15)

        self.cash_label = tk.Label(
            header,
            text="Available Cash: $0.00",
            bg="#172554",
            fg="#86efac",
            font=("Arial", 13, "bold")
        )
        self.cash_label.pack(side="right", padx=20)

    def create_interface(self):
        main = ttk.Frame(self.root, padding=10)
        main.pack(fill="both", expand=True)

        # Market watch
        left = ttk.Frame(main)
        left.pack(side="left", fill="both", expand=True, padx=(0, 10))

        ttk.Label(
            left,
            text="Market Watch",
            font=("Arial", 14, "bold")
        ).pack(anchor="w", pady=(0, 8))

        self.market_tree = ttk.Treeview(
            left,
            columns=("symbol", "name", "price", "change"),
            show="headings",
            height=10
        )

        for col, title, width in [
            ("symbol", "Symbol", 75),
            ("name", "Company", 150),
            ("price", "Price ($)", 95),
            ("change", "Change (%)", 95)
        ]:
            self.market_tree.heading(col, text=title)
            self.market_tree.column(col, width=width, anchor="center")

        self.market_tree.pack(fill="x")
        self.market_tree.bind(
            "<<TreeviewSelect>>",
            self.on_stock_selected
        )

        chart_frame = ttk.LabelFrame(
            left,
            text="Price Chart and Trend Estimate",
            padding=5
        )
        chart_frame.pack(fill="both", expand=True, pady=(10, 0))

        self.chart = tk.Canvas(
            chart_frame,
            height=230,
            bg="white",
            highlightthickness=1,
            highlightbackground="#cccccc"
        )
        self.chart.pack(fill="both", expand=True)
        self.chart.bind(
            "<Configure>",
            lambda event: self.draw_chart()
        )

        # Trading panel
        right = ttk.Frame(main, width=300)
        right.pack(side="right", fill="y")
        right.pack_propagate(False)

        ttk.Label(
            right,
            text="Trade Panel",
            font=("Arial", 14, "bold")
        ).pack(anchor="w", pady=(0, 8))

        trade_box = ttk.LabelFrame(
            right,
            text="Place Market Order",
            padding=12
        )
        trade_box.pack(fill="x")

        ttk.Label(
            trade_box,
            text="Selected Stock"
        ).pack(anchor="w")

        self.selected_label = ttk.Label(
            trade_box,
            text="AAPL",
            font=("Arial", 12, "bold")
        )
        self.selected_label.pack(anchor="w", pady=(0, 10))

        ttk.Label(trade_box, text="Quantity").pack(anchor="w")

        self.quantity_var = tk.StringVar(value="1")

        ttk.Entry(
            trade_box,
            textvariable=self.quantity_var
        ).pack(fill="x", pady=(3, 10))

        self.estimate_label = ttk.Label(
            trade_box,
            text="Estimated total: $0.00"
        )
        self.estimate_label.pack(anchor="w", pady=(0, 10))

        self.quantity_var.trace_add(
            "write",
            lambda *_: self.update_estimate()
        )

        buttons = ttk.Frame(trade_box)
        buttons.pack(fill="x")

        ttk.Button(
            buttons,
            text="BUY",
            command=lambda: self.place_order("BUY")
        ).pack(side="left", fill="x", expand=True, padx=(0, 5))

        ttk.Button(
            buttons,
            text="SELL",
            command=lambda: self.place_order("SELL")
        ).pack(side="left", fill="x", expand=True)

        ttk.Button(
            trade_box,
            text="Refresh Data",
            command=self.refresh_all
        ).pack(fill="x", pady=(8, 0))

        ttk.Button(
            trade_box,
            text="Export Records to Files",
            command=self.export_and_show
        ).pack(fill="x", pady=(8, 0))

        ttk.Button(
            trade_box,
            text="Open Data Folder",
            command=self.open_data_folder
        ).pack(fill="x", pady=(8, 0))

        account_box = ttk.LabelFrame(
            right,
            text="Account Summary",
            padding=10
        )
        account_box.pack(fill="x", pady=(10, 0))

        self.account_cash_label = ttk.Label(
            account_box,
            text="Cash: $0.00"
        )
        self.account_cash_label.pack(anchor="w", pady=3)

        self.holdings_value_label = ttk.Label(
            account_box,
            text="Holdings Value: $0.00"
        )
        self.holdings_value_label.pack(anchor="w", pady=3)

        self.total_value_label = ttk.Label(
            account_box,
            text="Total Account Value: $0.00",
            font=("Arial", 10, "bold")
        )
        self.total_value_label.pack(anchor="w", pady=3)

        # Notebook tabs
        tabs = ttk.Notebook(self.root)
        tabs.pack(fill="both", expand=False, padx=10, pady=(0, 10))

        portfolio_tab = ttk.Frame(tabs, padding=5)
        orders_tab = ttk.Frame(tabs, padding=5)
        trades_tab = ttk.Frame(tabs, padding=5)

        tabs.add(portfolio_tab, text="Portfolio")
        tabs.add(orders_tab, text="Orders")
        tabs.add(trades_tab, text="Trade History")

        self.portfolio_tree = self.make_table(
            portfolio_tab,
            [
                ("symbol", "Symbol", 100),
                ("quantity", "Quantity", 100),
                ("price", "Current Price", 130),
                ("value", "Market Value", 140)
            ]
        )

        self.orders_tree = self.make_table(
            orders_tab,
            [
                ("id", "Order ID", 65),
                ("time", "Time", 145),
                ("symbol", "Symbol", 75),
                ("side", "Side", 70),
                ("quantity", "Quantity", 75),
                ("price", "Price", 90),
                ("total", "Total", 90),
                ("status", "Status", 80)
            ]
        )

        self.trades_tree = self.make_table(
            trades_tab,
            [
                ("id", "Trade ID", 65),
                ("time", "Time", 145),
                ("symbol", "Symbol", 75),
                ("side", "Side", 70),
                ("quantity", "Quantity", 75),
                ("price", "Price", 90),
                ("total", "Total", 90)
            ]
        )

    def make_table(self, parent, columns):
        names = [item[0] for item in columns]

        tree = ttk.Treeview(
            parent,
            columns=names,
            show="headings",
            height=5
        )

        for name, heading, width in columns:
            tree.heading(name, text=heading)
            tree.column(name, width=width, anchor="center")

        scrollbar = ttk.Scrollbar(
            parent,
            orient="vertical",
            command=tree.yview
        )

        tree.configure(yscrollcommand=scrollbar.set)
        tree.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")

        return tree

    # ======================================
    # MARKET DATA
    # ======================================

    def get_stocks(self):
        conn = connect_db()
        cur = conn.cursor()

        cur.execute("""
            SELECT symbol, name, price, previous_price
            FROM stocks ORDER BY symbol
        """)

        rows = cur.fetchall()
        conn.close()
        return rows

    def get_price(self, symbol):
        conn = connect_db()
        cur = conn.cursor()

        cur.execute(
            "SELECT price FROM stocks WHERE symbol = ?",
            (symbol,)
        )

        row = cur.fetchone()
        conn.close()

        return float(row[0]) if row else 0.0

    def on_stock_selected(self, event=None):
        selection = self.market_tree.selection()

        if not selection:
            return

        values = self.market_tree.item(
            selection[0],
            "values"
        )

        if values:
            self.selected_symbol = values[0]
            self.selected_label.config(text=values[0])
            self.update_estimate()
            self.draw_chart()

    def update_market_prices(self):
        conn = connect_db()
        cur = conn.cursor()

        cur.execute("SELECT symbol, price FROM stocks")
        rows = cur.fetchall()

        for symbol, old_price in rows:
            movement = random.uniform(-0.008, 0.008)
            new_price = max(1.0, old_price * (1 + movement))

            cur.execute("""
                UPDATE stocks
                SET previous_price = ?, price = ?
                WHERE symbol = ?
            """, (old_price, new_price, symbol))

        conn.commit()
        conn.close()

        self.refresh_all()

        self.refresh_job = self.root.after(
            5000,
            self.update_market_prices
        )

    def refresh_all(self):
        self.refresh_market_table()
        self.refresh_portfolio()
        self.refresh_orders()
        self.refresh_trades()
        self.refresh_account_summary()
        self.update_estimate()
        self.draw_chart()
        export_records()

    def refresh_market_table(self):
        for item in self.market_tree.get_children():
            self.market_tree.delete(item)

        for symbol, name, price, previous in self.get_stocks():
            change = (
                (price - previous) / previous * 100
                if previous else 0
            )

            self.market_tree.insert(
                "",
                "end",
                values=(
                    symbol,
                    name,
                    f"{price:.2f}",
                    f"{change:+.2f}%"
                )
            )

    # ======================================
    # ACCOUNT
    # ======================================

    def get_cash(self):
        conn = connect_db()
        cur = conn.cursor()

        cur.execute(
            "SELECT value FROM settings WHERE key = 'cash'"
        )

        row = cur.fetchone()
        conn.close()

        return float(row[0]) if row else 0.0

    def set_cash(self, amount):
        conn = connect_db()
        cur = conn.cursor()

        cur.execute(
            "UPDATE settings SET value = ? WHERE key = 'cash'",
            (amount,)
        )

        conn.commit()
        conn.close()

    def get_quantity(self, symbol):
        conn = connect_db()
        cur = conn.cursor()

        cur.execute(
            "SELECT quantity FROM portfolio WHERE symbol = ?",
            (symbol,)
        )

        row = cur.fetchone()
        conn.close()

        return int(row[0]) if row else 0

    def set_quantity(self, symbol, quantity):
        conn = connect_db()
        cur = conn.cursor()

        if quantity <= 0:
            cur.execute(
                "DELETE FROM portfolio WHERE symbol = ?",
                (symbol,)
            )
        else:
            cur.execute("""
                INSERT INTO portfolio (symbol, quantity)
                VALUES (?, ?)
                ON CONFLICT(symbol)
                DO UPDATE SET quantity = excluded.quantity
            """, (symbol, quantity))

        conn.commit()
        conn.close()

    # ======================================
    # BUY AND SELL
    # ======================================

    def update_estimate(self):
        try:
            quantity = max(0, int(self.quantity_var.get()))
        except ValueError:
            quantity = 0

        total = quantity * self.get_price(self.selected_symbol)

        self.estimate_label.config(
            text=f"Estimated total: ${total:,.2f}"
        )

    def place_order(self, side):
        try:
            quantity = int(self.quantity_var.get())
        except ValueError:
            messagebox.showerror(
                "Invalid Quantity",
                "Please enter a whole number."
            )
            return

        if quantity <= 0:
            messagebox.showerror(
                "Invalid Quantity",
                "Quantity must be greater than zero."
            )
            return

        symbol = self.selected_symbol
        price = self.get_price(symbol)
        total = price * quantity
        cash = self.get_cash()
        owned = self.get_quantity(symbol)

        if side == "BUY":
            if total > cash:
                messagebox.showerror(
                    "Insufficient Funds",
                    f"Required: ${total:,.2f}\n"
                    f"Available: ${cash:,.2f}"
                )
                return

            self.set_quantity(symbol, owned + quantity)
            self.set_cash(cash - total)

        else:
            if quantity > owned:
                messagebox.showerror(
                    "Insufficient Shares",
                    f"You own only {owned} shares of {symbol}."
                )
                return

            self.set_quantity(symbol, owned - quantity)
            self.set_cash(cash + total)

        stamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        conn = connect_db()
        cur = conn.cursor()

        cur.execute("""
            INSERT INTO orders
            (time, symbol, side, quantity, price, total, status)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        """, (
            stamp, symbol, side, quantity,
            price, total, "FILLED"
        ))

        cur.execute("""
            INSERT INTO trades
            (time, symbol, side, quantity, price, total)
            VALUES (?, ?, ?, ?, ?, ?)
        """, (
            stamp, symbol, side, quantity, price, total
        ))

        conn.commit()
        conn.close()

        self.refresh_all()

        messagebox.showinfo(
            "Order Completed",
            f"{side} order completed!\n\n"
            f"Stock: {symbol}\n"
            f"Quantity: {quantity}\n"
            f"Price: ${price:,.2f}\n"
            f"Total: ${total:,.2f}\n\n"
            f"Saved in:\n{DATA_FOLDER}"
        )

    # ======================================
    # PORTFOLIO
    # ======================================

    def refresh_portfolio(self):
        for item in self.portfolio_tree.get_children():
            self.portfolio_tree.delete(item)

        conn = connect_db()
        cur = conn.cursor()

        cur.execute("""
            SELECT p.symbol, p.quantity, s.price
            FROM portfolio p
            JOIN stocks s ON p.symbol = s.symbol
            ORDER BY p.symbol
        """)

        rows = cur.fetchall()
        conn.close()

        for symbol, quantity, price in rows:
            self.portfolio_tree.insert(
                "",
                "end",
                values=(
                    symbol,
                    quantity,
                    f"${price:,.2f}",
                    f"${quantity * price:,.2f}"
                )
            )

    def refresh_account_summary(self):
        cash = self.get_cash()

        conn = connect_db()
        cur = conn.cursor()

        cur.execute("""
            SELECT p.quantity, s.price
            FROM portfolio p
            JOIN stocks s ON p.symbol = s.symbol
        """)

        rows = cur.fetchall()
        conn.close()

        holdings = sum(qty * price for qty, price in rows)
        total_value = cash + holdings

        self.cash_label.config(
            text=f"Available Cash: ${cash:,.2f}"
        )

        self.account_cash_label.config(
            text=f"Cash: ${cash:,.2f}"
        )

        self.holdings_value_label.config(
            text=f"Holdings Value: ${holdings:,.2f}"
        )

        self.total_value_label.config(
            text=f"Total Account Value: ${total_value:,.2f}"
        )

    # ======================================
    # ORDERS
    # ======================================

    def refresh_orders(self):
        for item in self.orders_tree.get_children():
            self.orders_tree.delete(item)

        conn = connect_db()
        cur = conn.cursor()

        cur.execute("""
            SELECT id, time, symbol, side, quantity,
                   price, total, status
            FROM orders ORDER BY id DESC LIMIT 100
        """)

        rows = cur.fetchall()
        conn.close()

        for row in rows:
            order_id, stamp, symbol, side, qty, price, total, status = row

            self.orders_tree.insert(
                "",
                "end",
                values=(
                    order_id, stamp, symbol, side, qty,
                    f"${price:,.2f}",
                    f"${total:,.2f}",
                    status
                )
            )

    def refresh_trades(self):
        for item in self.trades_tree.get_children():
            self.trades_tree.delete(item)

        conn = connect_db()
        cur = conn.cursor()

        cur.execute("""
            SELECT id, time, symbol, side, quantity, price, total
            FROM trades ORDER BY id DESC LIMIT 100
        """)

        rows = cur.fetchall()
        conn.close()

        for row in rows:
            trade_id, stamp, symbol, side, qty, price, total = row

            self.trades_tree.insert(
                "",
                "end",
                values=(
                    trade_id, stamp, symbol, side, qty,
                    f"${price:,.2f}",
                    f"${total:,.2f}"
                )
            )

    # ======================================
    # CHART AND TREND ESTIMATE
    # ======================================

    def draw_chart(self):
        if not hasattr(self, "chart"):
            return

        canvas = self.chart
        canvas.delete("all")

        width = canvas.winfo_width()
        height = canvas.winfo_height()

        if width < 100 or height < 100:
            return

        symbol = self.selected_symbol
        current_price = self.get_price(symbol)

        generator = random.Random(symbol)
        prices = []
        value = current_price * 0.94

        for _ in range(24):
            value *= 1 + generator.uniform(-0.012, 0.012)
            prices.append(value)

        prices[-1] = current_price

        left = 55
        right = 20
        top = 30
        bottom = 35

        plot_width = width - left - right
        plot_height = height - top - bottom

        if plot_width <= 0 or plot_height <= 0:
            return

        minimum = min(prices) * 0.99
        maximum = max(prices) * 1.01

        if maximum <= minimum:
            maximum = minimum + 1

        canvas.create_text(
            10, 10,
            anchor="nw",
            text=f"{symbol} simulated price",
            font=("Arial", 10, "bold"),
            fill="#172554"
        )

        for i in range(5):
            y = top + plot_height * i / 4
            label = maximum - (maximum - minimum) * i / 4

            canvas.create_line(
                left, y, width - right, y,
                fill="#e5e7eb"
            )

            canvas.create_text(
                left - 5, y,
                text=f"{label:.1f}",
                anchor="e",
                font=("Arial", 8)
            )

        points = []

        for i, price in enumerate(prices):
            x = left + plot_width * i / (len(prices) - 1)
            y = top + plot_height * (
                (maximum - price) / (maximum - minimum)
            )
            points.extend([x, y])

        canvas.create_line(
            *points,
            fill="#2563eb",
            width=2,
            smooth=True
        )

        recent = prices[-6:]
        trend = (recent[-1] - recent[0]) / 5
        forecast = current_price + trend * 3

        last_x = left + plot_width
        last_y = top + plot_height * (
            (maximum - current_price) / (maximum - minimum)
        )

        forecast_y = top + plot_height * (
            (maximum - max(minimum, min(maximum, forecast)))
            / (maximum - minimum)
        )

        canvas.create_line(
            last_x, last_y,
            width - right, forecast_y,
            fill="#f97316",
            width=2,
            dash=(5, 3)
        )

        canvas.create_text(
            width - 10,
            height - 10,
            anchor="se",
            text=f"Demo estimate: ${forecast:.2f}",
            fill="#c2410c",
            font=("Arial", 9, "bold")
        )

    # ======================================
    # EXPORT AND OPEN FOLDER
    # ======================================

    def export_and_show(self):
        try:
            export_records()
            messagebox.showinfo(
                "Export Successful",
                "All records have been exported.\n\n"
                f"Location:\n{DATA_FOLDER}\n\n"
                "Files:\n"
                "trade_history.csv\n"
                "orders.csv\n"
                "portfolio.csv\n"
                "stocks.csv\n"
                "account_balance.txt"
            )
        except OSError as error:
            messagebox.showerror(
                "Export Error",
                str(error)
            )

    def open_data_folder(self):
        try:
            os.startfile(DATA_FOLDER)
        except OSError as error:
            messagebox.showerror(
                "Folder Error",
                str(error)
            )

    # ======================================
    # CLOSE APPLICATION
    # ======================================

    def close_app(self):
        if self.refresh_job is not None:
            try:
                self.root.after_cancel(self.refresh_job)
            except tk.TclError:
                pass

        export_records()
        self.root.destroy()


# ==========================================
# RUN PROGRAM
# ==========================================

if __name__ == "__main__":
    initialize_database()

    root = tk.Tk()
    app = TradingApp(root)

    root.mainloop()
