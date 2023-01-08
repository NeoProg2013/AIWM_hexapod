package fragments;

import android.graphics.Color;
import android.os.Bundle;

import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;

import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.application.aiwm.ControlActivityViewModel;
import com.application.aiwm.R;
import com.application.aiwm.joystick.SquareJoystick;


public class DiagFragment extends Fragment {

    ControlActivityViewModel m_viewModel = null;

    final private String[] m_systemErrors = {
        "- Критическая ошибка",
        "- Внутренняя ошибка",
        "- Низкий заряд АКБ",
        "- Ошибка синхронизации",
        "- Математическая ошибка",
        "- Ошибка I2C шины",
    };
    final private String[] m_moduleErrors = {
        "- Сбой ядра передвижения",
        "- Сбой драйвера сервоприводов",
        "- Сбой подсистемы мониторинга",
        "- Сбой драйвера дисплея",
        "- Сбой подсистемы ориентации",
        "- Сбой подсистемы сенсоров",
    };

    public DiagFragment() {
        // Required empty public constructor
    }
    @Override public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        Log.e("DiagFragment", "call onCreateView");
        View root = inflater.inflate(R.layout.fragment_diag, container, false);

        m_viewModel = new ViewModelProvider(getActivity()).get(ControlActivityViewModel.class);

        root.findViewById(R.id.buttonDiagUpdate).setOnClickListener(new View.OnClickListener() {
            @Override public void onClick(View view) {
                LinearLayout l = root.findViewById(R.id.layoutSystemErrors);
                l.removeAllViews();

                int systemStatus = m_viewModel.swlp.getModuleStatus().getValue();
                for (int i = 0; i < m_systemErrors.length; ++i) {
                    int mask = 0x01 << i;
                    if ((systemStatus & mask) == mask) {
                        TextView t = new TextView(root.getContext());
                        t.setText(m_systemErrors[i]);
                        t.setTextColor(Color.RED);
                        l.addView(t);
                    }
                }


                l = root.findViewById(R.id.layoutModuleErrors);
                l.removeAllViews();

                int moduleStatus = m_viewModel.swlp.getModuleStatus().getValue();
                for (int i = 0; i < m_moduleErrors.length; ++i) {
                    int mask = 0x01 << i;
                    if ((moduleStatus & mask) == mask) {
                        TextView t = new TextView(root.getContext());
                        t.setText(m_moduleErrors[i]);
                        t.setTextColor(Color.RED);
                        l.addView(t);
                    }
                }
            }
        });
        return root;
    }

    @Override public void onDestroy() {
        super.onDestroy();
        Log.e("MotionFragment", "call onDestroy");
    }

    @Override public void onDestroyView() {
        super.onDestroyView();
        Log.e("MotionFragment", "call onDestroyView");
    }
}