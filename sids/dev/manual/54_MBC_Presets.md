# 54. MBC Presets

This document defines various pre-configured "Preset Boxes" within the Monistic Box Calculus (MBC) framework. These presets are reusable operational structures that combine fundamental MBC operators (`Bx_`) to demonstrate specific functionalities, instructional patterns, or common computational workflows. They serve as templates for more complex Box constructions.

---

## 54.1. Preset_Accumulator

*   **Preset ID:** `Preset_Accumulator`
*   **Type:** `preset_box`
*   **Description:** A simple instructional Box demonstrating accumulation via `Bx_Integrate`. Useful for teaching integration, continuity, and structural smoothing.
*   **Inputs:** `SemanticField`
*   **Outputs:** `IntegratedField`
*   **Operators Used:** `Bx_Integrate` (see `Section 53.3: Bx_Integrate`)
*   **Structure:**
    ```json
    {
      "steps": [
        {
          "action": "integrate",
          "operator": "Bx_Integrate",
          "input": "$in",
          "output": "$accum"
        }
      ]
    }
    ```
*   **Invariants:**
    *   **Continuity-Preservation:** No discontinuities allowed.
    *   **No-Synthetic-Classes:** Operator may not generate new semantic classes.
*   **Example Usage:**
    ```
    Input: Field({a,b,c})
    Output: ContinuousField(a→c)
    Note: Shows how proto-dimension formation emerges inside functional operators.
    ```

---

## 54.2. Preset_Amplifier

*   **Preset ID:** `Preset_Amplifier`
*   **Type:** `preset_box`
*   **Description:** Demonstrates semantic intensity amplification using `Bx_Amplify`. Ideal for showcasing `δ-deviation` prep and gradient enhancement.
*   **Inputs:** `SemanticField`
*   **Outputs:** `EnhancedField`
*   **Operators Used:** `Bx_Amplify` (see `Section 53.1: Bx_Amplify`)
*   **Structure:**
    ```json
    {
      "steps": [
        {
          "action": "amplify",
          "operator": "Bx_Amplify",
          "input": "$in",
          "output": "$enhanced"
        }
      ]
    }
    ```
*   **Invariants:**
    *   **Monotonicity:** Gradient ordering must remain preserved.
    *   **No-Polarity-Creation:** Amplifier may not create signed structures.
*   **Example Usage:**
    ```
    Input: Field(Δsmall)
    Output: Field(Δlarge)
    Note: Illustrates creation of deviation seeds for δ-tier operators.
    ```

---

## 54.3. Preset_Combiner

*   **Preset ID:** `Preset_Combiner`
*   **Type:** `preset_box`
*   **Description:** Demonstrates multi-channel identity-preserving summation via `Bx_SSum`. Often used for `ψ waveform` envelope introduction.
*   **Inputs:** `SemanticField[]` (Array of Semantic Fields)
*   **Outputs:** `CompositeField`
*   **Operators Used:** `Bx_SSum` (see `Section 53.6: Bx_SSum (Semantic Summation)`)
*   **Structure:**
    ```json
    {
      "steps": [
        {
          "action": "sum",
          "operator": "Bx_SSum",
          "input": "$in[]",
          "output": "$combined"
        }
      ]
    }
    ```
*   **Invariants:**
    *   **Identity-Preservation:** Inputs must remain individually recoverable.
    *   **No-Destructive-Interference:** Semantic cancellation is forbidden.
*   **Example Usage:**
    ```
    Input: [a, b, c]
    Output: Composite(a+b+c)
    Note: Demonstrates proto-superposition behavior inside a real Box.
    ```

---

## 54.4. Preset_FilterChain

*   **Preset ID:** `Preset_FilterChain`
*   **Type:** `preset_box`
*   **Description:** Two-stage semantic filtering using `Bx_Filter`. Useful for `Φ-projection` preparation, semantic isolation, and modal cleansing.
*   **Inputs:** `SemanticField`
*   **Outputs:** `FilteredField`
*   **Operators Used:** `Bx_Filter` (see `Section 53.2: Bx_Filter`)
*   **Structure:**
    ```json
    {
      "steps": [
        {
          "action": "filter",
          "operator": "Bx_Filter",
          "input": "$in",
          "mask": "Mask_A",
          "output": "$stage1"
        },
        {
          "action": "filter",
          "operator": "Bx_Filter",
          "input": "$stage1",
          "mask": "Mask_B",
          "output": "$filtered"
        }
      ]
    }
    ```
*   **Invariants:**
    *   **Identity-Safe:** Filtering must not delete semantic identity.
    *   **Monotonic-Attenuation:** Each stage must not amplify content.
*   **Example Usage:**
    ```
    Input: Field[F1,F2,F3]
    Output: Field[F2]
    Note: Basic demo of chained selective attenuation.
    ```

---

## 54.5. Preset_Normalizer

*   **Preset ID:** `Preset_Normalizer`
*   **Type:** `preset_box`
*   **Description:** Example normalization Box using `Bx_Transform` with a normalization rule. Demonstrates structural regularization inside a Box.
*   **Inputs:** `SemanticField`
*   **Outputs:** `NormalizedField`
*   **Operators Used:** `Bx_Transform` (see `Section 53.7: Bx_Transform`)
*   **Structure:**
    ```json
    {
      "steps": [
        {
          "action": "transform",
          "operator": "Bx_Transform",
          "rule": "NormalizeRule",
          "input": "$in",
          "output": "$norm"
        }
      ]
    }
    ```
*   **Invariants:**
    *   **Rewrite-Safety:** Normalization must follow rewrite constraints.
    *   **Dimensional-Stability:** Normalization must not increase dimension.
    *   **Semantic-Conservation:** Identity must be preserved.
*   **Example Usage:**
    ```
    Input: Field({0.2, 5.1, 10.7})
    Output: Field({0.02, 0.51, 1.00})
    Note: Illustrates rewrite-safe normalization inside an MBC Box.
    ```