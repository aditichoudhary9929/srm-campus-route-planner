/**
 * ============================================================================
 * map-data.js - Campus Layout Data & Visual Node Placement
 * SRM IST Delhi-NCR Campus (Modinagar, Ghaziabad)
 * ============================================================================
 */

const CAMPUS_CONFIG = {
    campusName: "SRM IST Delhi-NCR Campus",
    location: "Delhi-Meerut Road, Modinagar, Ghaziabad, UP 201204",
    viewBoxWidth: 1200,
    viewBoxHeight: 800,
    defaultSpeedMps: 1.25, // meters per second walking
    calorieFactor: 0.048,   // kcal per meter
    categories: {
        academic: { label: "Academic Blocks", color: "#38BDF8", bg: "rgba(56, 189, 248, 0.15)", icon: "🏛️" },
        admin:    { label: "Administration",  color: "#F59E0B", bg: "rgba(245, 158, 11, 0.15)", icon: "💼" },
        library:  { label: "Library",         color: "#10B981", bg: "rgba(16, 185, 129, 0.15)", icon: "📚" },
        lab:      { label: "Labs & Workshops", color: "#A78BFA", bg: "rgba(167, 139, 250, 0.15)", icon: "💻" },
        canteen:  { label: "Food & Canteens", color: "#FB923C", bg: "rgba(251, 146, 60, 0.15)", icon: "🍔" },
        hostel:   { label: "Hostels",         color: "#60A5FA", bg: "rgba(96, 165, 250, 0.15)", icon: "🛏️" },
        sports:   { label: "Sports & Arena",  color: "#4ADE80", bg: "rgba(74, 222, 128, 0.15)", icon: "⚽" },
        hospital: { label: "Medical Care",    color: "#F43F5E", bg: "rgba(244, 63, 94, 0.15)", icon: "🏥" },
        facility: { label: "Amenities & Halls", color: "#C084FC", bg: "rgba(192, 132, 252, 0.15)", icon: "✨" },
        parking:  { label: "Parking Lots",    color: "#94A3B8", bg: "rgba(148, 163, 184, 0.15)", icon: "🅿️" },
        gate:     { label: "Campus Gates",    color: "#FBBF24", bg: "rgba(251, 191, 36, 0.15)", icon: "🚪" }
    }
};

const DEFAULT_NODES = [
    { id: 0,  name: "Main_Gate",                 label: "Main Campus Gate",        type: "gate",     x: 130, y: 700, icon: "🚪", desc: "Main entry & security gate on Delhi-Meerut Highway" },
    { id: 1,  name: "Academic_Block_A",          label: "Academic Block A",        type: "academic", x: 330, y: 500, icon: "🏛️", desc: "CSE, IT, ECE lecture theatres & faculty chambers" },
    { id: 2,  name: "Academic_Block_B",          label: "Academic Block B",        type: "academic", x: 510, y: 530, icon: "🏢", desc: "Mechanical, Civil & Applied Sciences wing" },
    { id: 3,  name: "Central_Library",           label: "Central Library",         type: "library",  x: 370, y: 360, icon: "📚", desc: "3-floor library, digital repository & quiet study zones" },
    { id: 4,  name: "Administrative_Block",      label: "Administrative Block",    type: "admin",    x: 240, y: 630, icon: "💼", desc: "Dean Office, Accounts, Admission & Examination Cell" },
    { id: 5,  name: "Computer_Center",           label: "Computer Center",         type: "lab",      x: 480, y: 390, icon: "💻", desc: "High-performance computing labs, AI/ML servers" },
    { id: 6,  name: "Mechanical_Workshops",      label: "Mechanical Workshops",    type: "lab",      x: 620, y: 400, icon: "⚙️", desc: "Robotics, CNC milling, welding & foundry labs" },
    { id: 7,  name: "Central_Cafeteria",         label: "Central Cafeteria",       type: "canteen",  x: 490, y: 250, icon: "🍔", desc: "Main food court, vegetarian & multi-cuisine cafeteria" },
    { id: 8,  name: "Nescafe_Corner",            label: "Nescafe Corner",          type: "canteen",  x: 640, y: 240, icon: "☕", desc: "Student hangout hub, coffee, snacks & quick bites" },
    { id: 9,  name: "Tagore_Boys_Hostel",        label: "Tagore Boys Hostel",      type: "hostel",   x: 720, y: 110, icon: "🛏️", desc: "Senior boys residential wing with mess & gym" },
    { id: 10, name: "Raman_Boys_Hostel",         label: "Raman Boys Hostel",       type: "hostel",   x: 870, y: 110, icon: "🛏️", desc: "Junior boys hostel with recreational indoor lounge" },
    { id: 11, name: "Sarojini_Girls_Hostel",      label: "Sarojini Girls Hostel",   type: "hostel",   x: 810, y: 220, icon: "🏡", desc: "Girls residential block with 24/7 biometric security" },
    { id: 12, name: "Main_Sports_Ground",        label: "Main Sports Stadium",     type: "sports",   x: 980, y: 130, icon: "⚽", desc: "Cricket pavilion, athletic tracks & football ground" },
    { id: 13, name: "Basketball_Courts",         label: "Basketball Courts",       type: "sports",   x: 920, y: 270, icon: "🏀", desc: "Floodlit twin standard basketball courts" },
    { id: 14, name: "Indoor_Sports_Arena",       label: "Indoor Sports Arena",     type: "sports",   x: 1050, y: 310, icon: "🏸", desc: "Badminton, table tennis, squash & martial arts" },
    { id: 15, name: "Health_Care_Centre",        label: "Health Care Centre",      type: "hospital", x: 1090, y: 180, icon: "🏥", desc: "24/7 campus dispensary, ambulance & pharmacy" },
    { id: 16, name: "PNB_Bank_And_ATM",          label: "PNB Bank & ATM",          type: "facility", x: 150, y: 530, icon: "🏧", desc: "Punjab National Bank on-campus branch & ATM" },
    { id: 17, name: "University_Auditorium",      label: "University Auditorium",   type: "facility", x: 670, y: 540, icon: "🎭", desc: "1500-seat grand air-conditioned convention auditorium" },
    { id: 18, name: "Student_And_Staff_Parking", label: "Campus Parking Area",     type: "parking",  x: 80,  y: 570, icon: "🅿️", desc: "Designated multi-vehicle parking for cars & 2-wheelers" },
    { id: 19, name: "Temple_And_Meditation",     label: "Temple & Meditation",     type: "facility", x: 750, y: 660, icon: "🛕", desc: "Quiet botanical meditation garden & campus temple" },
    { id: 20, name: "Future_Expansion_Plot",     label: "Future Expansion Zone",   type: "facility", x: 920, y: 590, icon: "🏗️", desc: "Phase-II proposed Incubation & Research Park" }
];

const DEFAULT_EDGES = [
    { from: 0,  to: 1,  weight: 150 },
    { from: 0,  to: 4,  weight: 90  },
    { from: 0,  to: 18, weight: 50  },
    { from: 1,  to: 2,  weight: 100 },
    { from: 1,  to: 5,  weight: 50  },
    { from: 1,  to: 3,  weight: 80  },
    { from: 1,  to: 4,  weight: 110 },
    { from: 2,  to: 19, weight: 90  },
    { from: 2,  to: 17, weight: 70  },
    { from: 2,  to: 4,  weight: 130 },
    { from: 3,  to: 7,  weight: 100 },
    { from: 3,  to: 5,  weight: 90  },
    { from: 4,  to: 16, weight: 60  },
    { from: 5,  to: 6,  weight: 120 },
    { from: 6,  to: 7,  weight: 110 },
    { from: 7,  to: 17, weight: 120 },
    { from: 7,  to: 8,  weight: 60  },
    { from: 8,  to: 11, weight: 150 },
    { from: 8,  to: 9,  weight: 140 },
    { from: 9,  to: 11, weight: 180 },
    { from: 9,  to: 10, weight: 80  },
    { from: 10, to: 12, weight: 100 },
    { from: 11, to: 13, weight: 120 },
    { from: 12, to: 15, weight: 200 },
    { from: 12, to: 13, weight: 90  },
    { from: 13, to: 15, weight: 160 },
    { from: 13, to: 14, weight: 80  },
    { from: 14, to: 15, weight: 170 },
    { from: 16, to: 18, weight: 70  },
    { from: 17, to: 19, weight: 110 }
];

// Campus Zones for background overlay aesthetics
const CAMPUS_ZONES = [
    { name: "Academic Quadrangle", x: 280, y: 310, w: 400, h: 290, color: "rgba(56, 189, 248, 0.04)", border: "rgba(56, 189, 248, 0.2)" },
    { name: "Student Living Hub", x: 670, y: 70,  w: 250, h: 210, color: "rgba(96, 165, 250, 0.04)", border: "rgba(96, 165, 250, 0.2)" },
    { name: "Sports & Athletics Complex", x: 890, y: 80, w: 260, h: 280, color: "rgba(74, 222, 128, 0.04)", border: "rgba(74, 222, 128, 0.2)" },
    { name: "Gateway & Administrative Hub", x: 40, y: 480, w: 270, h: 270, color: "rgba(245, 158, 11, 0.04)", border: "rgba(245, 158, 11, 0.2)" }
];

if (typeof module !== 'undefined' && module.exports) {
    module.exports = { CAMPUS_CONFIG, DEFAULT_NODES, DEFAULT_EDGES, CAMPUS_ZONES };
}
