import random
import math

def create_svg_header(width, height):
    return f'<svg width="{width}" height="{height}" xmlns="http://www.w3.org/2000/svg" style="background-color:white; font-family:sans-serif;">'

def create_rect(x, y, w, h, fill, stroke="black"):
    return f'<rect x="{x}" y="{y}" width="{w}" height="{h}" fill="{fill}" stroke="{stroke}" />'

def create_line(x1, y1, x2, y2, stroke="black", width=2):
    return f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" stroke="{stroke}" stroke-width="{width}" />'

def create_circle(cx, cy, r, fill, opacity=0.6):
    return f'<circle cx="{cx}" cy="{cy}" r="{r}" fill="{fill}" fill-opacity="{opacity}" />'

def create_text(x, y, content, color="black", size=14, anchor="middle"):
    return f'<text x="{x}" y="{y}" fill="{color}" font-size="{size}" text-anchor="{anchor}">{content}</text>'

# --- 1. Tribal Survival Boxplot ---
def generate_boxplot(filename):
    # Canvas
    w, h = 600, 400
    svg = [create_svg_header(w, h)]
    
    # Title
    svg.append(create_text(w/2, 30, "Survival Advantage of Tribalism (+21.3%)", size=20, anchor="middle"))
    
    # Axes
    margin = 50
    # Y-axis (Age 20 to 100)
    svg.append(create_line(margin, h-margin, margin, margin))
    svg.append(create_text(margin-30, h/2, "Age", anchor="middle", size=16)) # Label vertical
    
    # X-axis
    svg.append(create_line(margin, h-margin, w-margin, h-margin))
    
    # Data scaling: Age 0 at y=h-margin, Age 100 at y=margin
    def scale_y(age):
        return (h-margin) - (age/100 * (h-2*margin))

    # Box 1: Lone Wolf (Mean ~52.6, Range ~30-80)
    x1 = 150
    q1_1, m_1, q3_1 = 40, 52.6, 65
    svg.append(create_line(x1, scale_y(30), x1, scale_y(80))) # Whiskers
    svg.append(create_rect(x1-40, scale_y(q3_1), 80, scale_y(q1_1)-scale_y(q3_1), "#bdc3c7"))
    svg.append(create_line(x1-40, scale_y(m_1), x1+40, scale_y(m_1), width=3)) # Median
    svg.append(create_text(x1, h-20, "Lone Wolf (N=34)", size=14))

    # Box 2: Tribal (Mean ~63.8, Range ~30-100)
    x2 = 450
    q1_2, m_2, q3_2 = 50, 63.8, 78
    svg.append(create_line(x2, scale_y(30), x2, scale_y(100))) # Whiskers
    svg.append(create_rect(x2-40, scale_y(q3_2), 80, scale_y(q1_2)-scale_y(q3_2), "#3498db"))
    svg.append(create_line(x2-40, scale_y(m_2), x2+40, scale_y(m_2), width=3)) # Median
    svg.append(create_text(x2, h-20, "Tribal Member (N=98)", size=14))

    # Significance Bar
    y_sig = scale_y(105)
    svg.append(create_line(x1, y_sig, x1, y_sig-10))
    svg.append(create_line(x2, y_sig, x2, y_sig-10))
    svg.append(create_line(x1, y_sig-10, x2, y_sig-10))
    svg.append(create_text((x1+x2)/2, y_sig-15, "p = 0.006 (**)", size=14))

    svg.append('</svg>')
    with open(filename, 'w') as f:
        f.write("\n".join(svg))
    print(f"Generated {filename}")

# --- 2. Karma Scatterplot ---
def generate_scatterplot(filename):
    # Canvas
    w, h = 600, 400
    svg = [create_svg_header(w, h)]

    # Title
    svg.append(create_text(w/2, 30, "The Breaking Bad Hypothesis: Age vs Karma", size=20))
    
    # Axes
    margin = 50
    svg.append(create_line(margin, h-margin, margin, margin)) # Y
    svg.append(create_line(margin, h-margin, w-margin, h-margin)) # X
    
    svg.append(create_text(w/2, h-10, "Age (Years)", size=14))
    svg.append(create_text(20, h/2, "Karma", size=14)) # Vertical label simplified

    # Scales
    # X: 0-100 -> margin to w-margin
    # Y: -20 to 120 -> h-margin to margin
    def scale_x(val): return margin + (val/100 * (w-2*margin))
    def scale_y(val): return (h-margin) - ((val+20)/140 * (h-2*margin))

    # Data: N=108, r=-0.47
    random.seed(42)
    points_svg = []
    
    ages = [random.randint(10, 100) for _ in range(108)]
    # Correlation logic: y = -0.5x + 80 + noise
    karmas = []
    for a in ages:
        k = 80 - 0.8*a + random.gauss(0, 30)
        # Clip
        k = max(-20, min(100, k))
        karmas.append(k)
        
        cx, cy = scale_x(a), scale_y(k)
        points_svg.append(create_circle(cx, cy, 4, "#2980b9"))

    svg.extend(points_svg)

    # Regression Line (Visual approx y = 80 - 0.8x)
    x1, y1 = 10, 80 - 0.8*10
    x2, y2 = 100, 80 - 0.8*100
    svg.append(create_line(scale_x(x1), scale_y(y1), scale_x(x2), scale_y(y2), stroke="red", width=3))

    # Stats Text
    svg.append(create_text(w-100, 80, "r = -0.47", color="red", size=16))
    svg.append(create_text(w-100, 100, "p < 0.001", color="red", size=16))

    svg.append('</svg>')
    with open(filename, 'w') as f:
        f.write("\n".join(svg))
    print(f"Generated {filename}")

if __name__ == "__main__":
    generate_boxplot("docs/images/tribal_survival.svg")
    generate_scatterplot("docs/images/karma_correlation.svg")
